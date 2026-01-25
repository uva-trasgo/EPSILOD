/**
 * @file poisson_kernels.c
 * @brief Example for poisson equation. Kernel code.
 *
 * @copyright This software is part of the EPSILOD project by Trasgo Group, UVa.
 * The relevant license, warranty and copyright notice is available in the EPSILOD project repository.
 */

#include <math.h>

#include "poisson_ext_type.h"
#include "epsilod_kernels.h"

#if EPSILOD_IS_FLOAT(EPSILOD_BASE_TYPE)
#define POW(a, b) powf(a, b)
#else // !EPSILOD_IS_FLOAT(EPSILOD_BASE_TYPE)
#define POW(a, b) pow(a, b)
#endif // EPSILOD_IS_FLOAT(EPSILOD_BASE_TYPE)

/* KERNEL GENERIC: POISSON */
CTRL_KERNEL(noop_poisson, GENERIC, DEFAULT, KHitTile(EPSILOD_BASE_TYPE) matrix, const KHitTile(EPSILOD_BASE_TYPE) matrixCopy, EpsilodCoords global_coords, KHitTile_float stencil, float factor, const Epsilod_ext ext_params, {
	hit(matrix, thr_i, thr_j) = hit(matrixCopy, thr_i, thr_j);
});

/* KERNEL GENERIC: POISSON */
CTRL_KERNEL(updateCell_poisson, GENERIC, DEFAULT, KHitTile(EPSILOD_BASE_TYPE) matrix, const KHitTile(EPSILOD_BASE_TYPE) matrixCopy, EpsilodCoords global_coords, KHitTile_float stencil, float factor, const Epsilod_ext ext_params, {
	int x = thr_i;
	int y = thr_j;

	EPSILOD_BASE_TYPE b  = ext_params.b;
	EPSILOD_BASE_TYPE dx = ext_params.dx;
	EPSILOD_BASE_TYPE dy = ext_params.dy;
	// vec2i size = ext_params.size;

	// if (x == 0 && y == 0)
	// 	printf("Condition: %d %d\n", size.x / 4, size.y / 4);

	// printf("Thread: %d %d\n", x, y);

	int x_g = thr_i + global_coords.offset[0];
	int y_g = thr_j + global_coords.offset[1];

	// printf("Global: %d %d\n", x_g, y_g);

	if (x_g == global_coords.size[0] / 4 &&
		y_g == global_coords.size[1] / 4)
		b = 2500.;
	else if (x_g == 3 * global_coords.size[0] / 4 &&
			 y_g == 3 * global_coords.size[1] / 4)
		b = -2500.;

	hit(matrix, x, y) = (POW(dy, 2) *
							 (hit(matrixCopy, x - 1, y) +
							  hit(matrixCopy, x + 1, y)) +
						 POW(dx, 2) *
							 (hit(matrixCopy, x, y - 1) +
							  hit(matrixCopy, x, y + 1)) -
						 b * POW(dx, 2) * POW(dy, 2)) /
						(2 * (POW(dx, 2) + POW(dy, 2)));
});
