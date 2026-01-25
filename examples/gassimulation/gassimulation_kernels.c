/**
 * @file gassimulation_kernels.c
 * @brief Example for gas simulation
 * 	Translated from Muesli example code
 * 	Kernel code
 *
 * @copyright This software is part of the EPSILOD project by Trasgo Group, UVa.
 * The relevant license, warranty and copyright notice is available in the EPSILOD project repository.
 */

#include "gassimulation_types.h"
#include "gassimulation_ext_type.h"

#include "epsilod_kernels.h"

CTRL_KERNEL(initCell_gassimulation, GENERIC, DEFAULT, KHitTile_cell_t matrix, EpsilodCoords global_coords, Epsilod_ext ext_params, {
	cell_t c = ((cell_t){{0.}});
	// printf("Thr: %d, %d, %d\n", thr_i, thr_j, thr_k );

	const vec3f                   *offsets   = ext_params.offsets;
	const GASSIMULATION_CELL_TYPE *wis       = ext_params.wis;
	GASSIMULATION_CELL_TYPE        cellwidth = ext_params.cellwidth;

	// Global coordinates
	// Switch j and i to mimic muesli
	// Subtract the stencil radius as we have added an extra border to de matrix
	int    radius = 1;
	HitInd x      = thr_j + global_coords.offset[1] - radius;
	HitInd y      = thr_i + global_coords.offset[0] - radius;
	HitInd z      = thr_k + global_coords.offset[2] - radius;
	// Subtract double the stencil radius as we have added an extra border to de matrix
	int size_x = global_coords.size[0] - 2 * radius;
	int size_y = global_coords.size[1] - 2 * radius;
	int size_z = global_coords.size[2] - 2 * radius;

	// Initialise the inner part; the extra border remains at 0 for the entire cell
	// Takes into account the thread swap i, j
	if (x >= 0 && y >= 0 && z >= 0 && x < size_y && y < size_x && z < size_z) {

		for (int i = 0; i < Q; i++) {
			GASSIMULATION_CELL_TYPE wi = wis[i];
			GASSIMULATION_CELL_TYPE cw = cellwidth;
			vec3f                   v  = ((vec3f){.1f, 0, 0});
			vec3f                   scaled;
			VEC3_SCALE(scaled, offsets[i], cw);
			GASSIMULATION_CELL_TYPE dot = VEC3_DOT(scaled, v);
			// float dot = offsets[i].x * 0.1f;
			c.data[i] = wi * 1.f * (1 + (1 / (cw * cw)) * (3 * dot + (9 / (2 * cw * cw)) * dot * dot - (3.f / 2) * VEC3_DOT(v, v)));
			// c.data[i] = wi * 1.f * (1 + (1 / (cw * cw)) * (3 * dot + (9 / (2 * cw * cw)) * dot * dot - (3.f / 2) * 0.01f));
			// c.data[i] = i % 10 + (float)x / 100 + (float)y / 10000 + (float)z / 1000000;
		}

		// This boolean emulates an unexpected behaviour in muesli's example
		// When executed on gpu, power funcion receiving a negative base returns NaN, despite the exponent being an integer
		// This causes the power part of the condition to be false under those circumstances
		bool bases_positive = true; // x - 50 >= 0 && y - 50 >= 0 && z - 8 >= 0;
		if (x <= 1 || y <= 1 || z <= 1 || x >= size_x - 2 || y >= size_y - 2 || z >= size_z - 2
			// @arturo TODO: No linka con rintf, pero creo que está -lm puesto.... revisar
			|| (bases_positive && rintf(rintf(POW(x - 50, 2)) + rintf(POW(y - 50, 2)) + rintf(POW(z - 8, 2))) <= 225)) {
			// c.data[0]         = 0.;
			// floatparts *parts = (floatparts *)c.data;
			// parts->sign       = 0;
			// parts->exponent   = MAX_EXPONENT;

			// @arturo
			// TODO: REvisar esta condición, estamos dentro de un condicional que dice casi
			// lo mismo, excepto que aquí sólo afecta a los bordes finales si son de la parte
			// extendida extra... o es -2 y sobra el condicional, o algo es diferente entre
			// los bordes iniciales y finales, raro, raro.
			if (x <= 1 || x >= size_x - 1 || y <= 1 || y >= size_y - 1 || z <= 1 || z >= size_z - 1) {
				// parts->mantissa = 1 << (MANTISSA_SIZE - 1) | FLAG_KEEP_VELOCITY;
				c.data[0] = INFINITY;
			} else {
				// parts->mantissa = 1 << (MANTISSA_SIZE - 1) | FLAG_OBSTACLE;
				c.data[0] = -INFINITY;
			}
		}

	} // Extra border condition

	hit(matrix, thr_i, thr_j, thr_k) = c;
});

/* KERNEL GENERIC: GAS SIMULATION */
CTRL_KERNEL(updateCell_gassimulation, GENERIC, DEFAULT, KHitTile_cell_t matrix, const KHitTile_cell_t matrixCopy, EpsilodCoords global_coords, KHitTile_float stencil, float factor, const Epsilod_ext ext_params, {
	int x = thr_i;
	int y = thr_j;
	int z = thr_k;

	const vec3f                   *offsets   = ext_params.offsets;
	const unsigned char           *opposite  = ext_params.opposite;
	const GASSIMULATION_CELL_TYPE *wis       = ext_params.wis;
	GASSIMULATION_CELL_TYPE        cellwidth = ext_params.cellwidth;
	GASSIMULATION_CELL_TYPE        deltaT    = ext_params.deltaT;
	GASSIMULATION_CELL_TYPE        tau       = ext_params.tau;

	cell_t cell = hit(matrixCopy, x, y, z);

	if (cell.data[0] == FLAG_KEEP_VELOCITY) {
		hit(matrix, x, y, z) = cell;
		// return cell;
	} else {
		// Streaming.
		for (int i = 1; i < Q; i++) {
			// Switch offsets x and y to mimic muesli
			int    sx            = x + (int)(offsets[i].y);
			int    sy            = y + (int)(offsets[i].x);
			int    sz            = z + (int)(offsets[i].z);
			cell_t cellStreaming = hit(matrixCopy, sx, sy, sz);
			cell.data[i]         = cellStreaming.data[i];
		}

		// Collision.
		if (cell.data[0] == FLAG_OBSTACLE) {
			cell_t cell2 = cell;
			for (size_t i = 1; i < Q; i++) {
				cell.data[i] = cell2.data[opposite[i]];
			}
			hit(matrix, x, y, z) = cell;
		} else {
			GASSIMULATION_CELL_TYPE p  = 0;
			vec3f                   vp = {0}; // We initialize this way to prevent compilation errors due to commas
			for (size_t i = 0; i < Q; i++) {
				p += cell.data[i];
				vec3f scaled;
				VEC3_SCALE(scaled, offsets[i], cellwidth)
				VEC3_SCALE(scaled, scaled, cell.data[i])
				VEC3_ADD(vp, scaled)
			}
			vec3f v;
			if (p == 0) {
				v = vp;
			} else {
				VEC3_SCALE(v, vp, (1 / p))
			}
			for (size_t i = 0; i < Q; i++) {

				// feq function inline
				GASSIMULATION_CELL_TYPE wi = wis[i];
				GASSIMULATION_CELL_TYPE c  = cellwidth;
				vec3f                   scaled;
				VEC3_SCALE(scaled, offsets[i], c)
				GASSIMULATION_CELL_TYPE dot = VEC3_DOT(scaled, v);
				GASSIMULATION_CELL_TYPE feq = wi * p * (1 + (1 / (c * c)) * (3 * dot + (9 / (2 * c * c)) * dot * dot - (3.f / 2) * VEC3_DOT(v, v)));

				cell.data[i] = cell.data[i] + deltaT / tau * (feq - cell.data[i]);
			}
			hit(matrix, x, y, z) = cell;
		}
	}
});
