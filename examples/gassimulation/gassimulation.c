/**
 * @file gassimulation.c
 * @brief Example for gas simulation.
 *
 * @copyright This software is part of the EPSILOD project by Trasgo Group, UVa.
 * The relevant license, warranty and copyright notice is available in the EPSILOD project repository.
 *
 * @copyright This file is part of a modified version of a Muesli example. Thus the following applies:
 * @copyright Copyright (c) 2016
 * 		Steffen Ernsting <s.ernsting@uni-muenster.de>,
 * 		Herbert Kuchen <kuchen@uni-muenster.de>,
 * 		Fabian Wrede <fabian.wrede@wi.uni-muenster.de>.
 *
 * @copyright Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * @copyright The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 */

#include <stdio.h>
#include <stdlib.h>

#include <gassimulation_types.h>
#include <gassimulation_ext_type.h>

#include <epsilod.h>

GASSIMULATION_CELL_TYPE deltaT    = 1.f;
GASSIMULATION_CELL_TYPE tau       = 0.65;
GASSIMULATION_CELL_TYPE cellwidth = 1.0f;

vec3f offsets[Q] = {
	{0, 0, 0},   // 0
	{-1, 0, 0},  // 1
	{1, 0, 0},   // 2
	{0, -1, 0},  // 3
	{0, 1, 0},   // 4
	{0, 0, -1},  // 5
	{0, 0, 1},   // 6
	{-1, -1, 0}, // 7
	{-1, 1, 0},  // 8
	{1, -1, 0},  // 9
	{1, 1, 0},   // 10
	{-1, 0, -1}, // 11
	{-1, 0, 1},  // 12
	{1, 0, -1},  // 13
	{1, 0, 1},   // 14
	{0, -1, -1}, // 15
	{0, -1, 1},  // 16
	{0, 1, -1},  // 17
	{0, 1, 1}};  // 18

unsigned char opposite[Q] = {
	0,
	2, 1, 4, 3, 6, 5,
	10, 9, 8, 7, 14, 13, 12, 11, 18, 17, 16, 15};

GASSIMULATION_CELL_TYPE wis[Q] = {
	1.f / 3,
	1.f / 18,
	1.f / 18,
	1.f / 18,
	1.f / 18,
	1.f / 18,
	1.f / 18,
	1.f / 36,
	1.f / 36,
	1.f / 36,
	1.f / 36,
	1.f / 36,
	1.f / 36,
	1.f / 36,
	1.f / 36,
	1.f / 36,
	1.f / 36,
	1.f / 36,
	1.f / 36};

/* Calls to these three functions can be substituted by the corresponding macros defined in gassimulation_types.h */

vec3f vec3Scale(vec3f v, const float val) {
	v.x *= val;
	v.y *= val;
	v.z *= val;
	return v;
}

float vec3Dot(const vec3f *v1, const vec3f *v2) {
	return v1->x * v2->x + v1->y * v2->y + v1->z * v2->z;
}

vec3f vec3Add(const vec3f *v1, const vec3f *v2) {
	vec3f v = *v1;
	v.x += v2->x;
	v.y += v2->y;
	v.z += v2->z;
	return v;
}

float feq(size_t i, float p, const vec3f *v) {
	float wi     = wis[i];
	float c      = cellwidth;
	vec3f scaled = vec3Scale(offsets[i], c);
	float dot    = vec3Dot(&scaled, v);
	return wi * p * (1 + (1 / (c * c)) * (3 * dot + (9 / (2 * c * c)) * dot * dot - (3.f / 2) * vec3Dot(v, v)));
}

char *input_file_name;
char *output_file_name;
vec3l size;

// void initCell(int x, int y, int z, vec3i size, cell_t *c) {

// 	for (int i = 0; i < Q; i++) {
// 		float wi     = wis[i];
// 		float cw     = cellwidth;
// 		vec3f v      = {.1f, 0, 0};
// 		vec3f scaled = vec3Scale(offsets[i], cw);
// 		float dot    = vec3Dot(&scaled, &v);
// 		c->data[i]   = wi * 1.f * (1 + (1 / (cw * cw)) * (3 * dot + (9 / (2 * cw * cw)) * dot * dot - (3.f / 2) * vec3Dot(&v, &v)));
// 		// c->data[i]   = i % 10 + (float)x / 100 + (float)y / 10000 + (float)z / 1000000;
// 	}
// 	// return;

// 	// if (x == 40 && y == 39 && z == 6) {
// 	// int   base_x = x - 50;
// 	// int   base_y = y - 50;
// 	// int   base_z = z - 8;
// 	// float pow_x  = POW(x - 50, 2);
// 	// float pow_y  = POW(y - 50, 2);
// 	// float pow_z  = POW(z - 8, 2);
// 	// float power  = POW(x - 50, 2) + POW(y - 50, 2) + POW(z - 8, 2);
// 	// printf("**** bases %d %d %d | %.6f %.6f %.6f Power: %.6f\n", base_x, base_y, base_z, pow_x, pow_y, pow_z, power);
// 	//}

// 	// 40 39 6 -> 100 + 121 + 4 = 225
// 	// 40 39 7 -> 100 + 121 + 1 = 222
// 	// 40 39 8 -> 100 + 121 + 0 = 221
// 	// 40 39 9 -> 100 + 121 + 1 = 222
// 	// 40 39 10 -> 100 + 121 + 4 = 225
// 	// This condition emulates an unexpected behaviour in muesli's example
// 	// When executed on gpu, power funcion receiving a negative base returns NaN, despite the exponent being an integer
// 	// This causes the power part of the condition to be false under those circumstances
// 	bool bases_positive = x - 50 >= 0 && y - 50 >= 0 && z - 8 >= 0;
// 	if (x <= 1 || y <= 1 || z <= 1 || x >= size.x - 2 || y >= size.y - 2 || z >= size.z - 2 || (bases_positive && POW(x - 50, 2) + POW(y - 50, 2) + POW(z - 8, 2) <= 225)) {

// 		floatparts *parts = (floatparts *)&c->data[0];
// 		parts->sign       = 0;
// 		parts->exponent   = 255;
// 		if (x <= 1 || x >= size.x - 1 || y <= 1 || y >= size.y - 1 || z <= 1 || z >= size.z - 1) {
// 			parts->mantissa = 1 << 22 | FLAG_KEEP_VELOCITY;
// 		} else {
// 			parts->mantissa = 1 << 22 | FLAG_OBSTACLE;
// 		}
// 	}
// 	// return c;
// }

// /* A. INITIALIZE ARRAY: GENERATE */
// void initData(HitTile_cell_t tileMat, EpsilodCoords global, Epsilod_ext ext_params) {
// 	int radius = 1;

// 	cell_t c = {{0.f}};
// 	for (int i = 0; i < tileMat.acumCard; i++)
// 		hit(tileMat, i) = c;

// 	for (int i = 0; i < size.x; i++) {
// 		for (int j = 0; j < size.y; j++) {
// 			for (int k = 0; k < size.z; k++) {
// 				if (
// 					hit_sigIn(hit_tileDimSig(tileMat, 0), i + radius) &&
// 					hit_sigIn(hit_tileDimSig(tileMat, 1), j + radius) &&
// 					hit_sigIn(hit_tileDimSig(tileMat, 2), k + radius)) {
// 					cell_t *cell = &hit(
// 						tileMat,
// 						i + radius - hit_tileDimBegin(tileMat, 0),
// 						j + radius - hit_tileDimBegin(tileMat, 1),
// 						k + radius - hit_tileDimBegin(tileMat, 2));
// 					// Switch j and i to mimic muesli
// 					initCell(j, i, k, size, cell);
// 				}
// 			}
// 		}
// 	}
// }

/* B. WRITE RESULTS */
void outputData(HitTile_cell_t io_tile, Epsilod_ext *ext_params) {

	if (strcmp(output_file_name, "-") == 0) {
		if (hit_Rank == 0) {
			printf("Gas simulation not writing output.\n");
			fflush(stdout);
		}
		return;
	}

	int radius = 1;

	/* DATA PART */
	/* Create a virtual tile with the real shape to use as reference */
	HitShape shp_root = hitShape(
		(radius, size.x + radius - 1),
		(radius, size.y + radius - 1),
		(radius, size.z + radius - 1));
	HitTile_cell_t virtual_root;
	hit_tileDomainShape(&virtual_root, cell_t, shp_root);
	io_tile.ref = &virtual_root;

	/* Select the local data part without borders */
	HitShape shp_data = hitShape(
		(radius, size.x + radius - 1),
		(radius, size.y + radius - 1),
		(radius, size.z + radius - 1));
	HitShape       shp_local_data = hit_shapeIntersect(shp_data, hit_tileShape(io_tile));
	HitTile_cell_t data;
	hit_tileSelectArrayCoords(&data, &io_tile, shp_local_data);

	/* Write distributed file */
	// var, file, coord, datatype, s1, s2
	// hit_tileTextFileWrite(&data, output_file_name, HIT_FILE_ARRAY, HIT_FILE_CELL_T, 6, 4);
	// var, fileNamePrefix, fileNameSuffix, fileRank, format, coord, header, datatype, formatSize1, formatSize2
	// ARTURO: Cambio de HIT_FILE_CELL_T a TEXT para que compile sin quejarse. Necesito nueva
	// versión de hitmap
	hit_tileFileWriteOptions(&data, output_file_name, NULL, HIT_FILE_RUNTIME, HIT_FILE_BINARY, HIT_FILE_ARRAY, HIT_FILE_NO_HEADER, HIT_FILE_TYPE_UNKNOWN, 1, 0);
	// hit_tileFileWriteOptions(&data, output_file_name, NULL, HIT_FILE_RUNTIME, HIT_FILE_TEXT, HIT_FILE_TILE, HIT_FILE_NO_HEADER, HIT_FILE_CELL_T, 8, 6);
	// hit_tileFileWriteOptions(&data, output_file_name, NULL, HIT_FILE_RUNTIME, HIT_FILE_TEXT, HIT_FILE_TILE, HIT_FILE_NO_HEADER, HIT_FILE_TEXT, 8, 6);
}

/* D. DECLARATIONS OF OPTIMIZED STENCIL KERNEL
 * SEE gassimulation_kernels.c FILE */
REGISTER_STENCIL(updateCell_gassimulation, GENERIC, DEFAULT);
REGISTER_INIT(initCell_gassimulation, GENERIC, DEFAULT);

/* HELP: PRINT ARGUMENT USAGE */
void printUsage(char *argv[]) {
	if (hit_Rank == 0) {
		fprintf(stderr, "\n=== EPSILOD EXAMPLE: Gas simulation ===\n");
		fprintf(stderr, "\nUsage: %s <size_x> <size_y> <size_z> <numIterations> <output_file> <device_selection_file>\n", argv[0]);
		fprintf(stderr, "\n");
	}
}

/* MAIN: STENCIL PROGRAM, READ ARGUMENTS AND CALL THE PATTERN */
int main(int argc, char *argv[]) {
	/* Init communication system */
	Ctrl_Init(&argc, &argv);

	/* Check program arguments number */
	if (argc != 7) {
		printUsage(argv);
		exit(EXIT_FAILURE);
	}

	/* READ ARGUMENTS */
	size.x                      = atol(argv[1]);
	size.y                      = atol(argv[2]);
	size.z                      = atol(argv[3]);
	int iterations              = atoi(argv[4]);
	output_file_name            = argv[5];
	char *device_selection_file = argv[6];

	/* STENCIL DECLARATION */
	/* RADIUS */
	int      radius                    = 1;
	HitShape shp_stencil_gassimulation = hitShape((-radius, radius), (-radius, radius), (-radius, radius));

	/* WEIGHTS: SPECIALIZED KERNEL, VALUES ARE USED ONLY TO COMPUTE BORDERS */
	// int    stencilDataSize = (2 * radius + 1) * (2 * radius + 1) * (2 * radius + 1);
	// cell_t stencilData_gassimulation[stencilDataSize];
	/*for (int i = 0; i < stencilDataSize; i++)
		for (int j = 0; j < Q; j++)
			stencilData_gassimulation[i].data[j] = 1; // Non-Zero arbitrary value
	*/
	float stencilData_gassimulation[] = {
		// x = 0, z -->
		0, 1, 0,
		1, 1, 1,
		0, 1, 0,
		// x = 1
		1, 1, 1,
		1, 1, 1,
		1, 1, 1,
		// x = 2
		0, 1, 0,
		1, 1, 1,
		0, 1, 0};

	/* POINTER TO SPECIFIC KERNEL */
	stencilDeviceFunction  f_stencil = updateCell_gassimulation;
	initDataDeviceFunction f_init    = initCell_gassimulation;

	HitInd sizes[3] = {size.x + (radius * 2), size.y + (radius * 2), size.z + (radius * 2)};

	/* LAUNCH STENCIL COMPUTATION */
	Epsilod_ext ext_params = {
		//.offsets   = offsets,
		//					  .opposite  = opposite,
		//					  .wis       = wis,
		.cellwidth = cellwidth,
		.deltaT    = deltaT,
		.tau       = tau};
	memcpy(ext_params.offsets, offsets, Q * sizeof(vec3f));
	memcpy(ext_params.opposite, opposite, Q * sizeof(unsigned char));
	memcpy(ext_params.wis, wis, Q * sizeof(GASSIMULATION_CELL_TYPE));
	stencilComputation(sizes, shp_stencil_gassimulation, stencilData_gassimulation, 1.0f, iterations, NULL, f_init, NULL, f_stencil, outputData, &ext_params, device_selection_file);
	// stencilComputation(sizes, shp_stencil_gassimulation, stencilData_gassimulation, 1.0f, iterations, initData, NULL, f_stencil, outputData, &ext_params, device_selection_file);
	// stencilComputation(sizes, shp_stencil_gassimulation, stencilData_gassimulation, 1.0f, iterations, initData, f_init, f_stencil, outputData, &ext_params, device_selection_file);

	/* END */
	Ctrl_Finalize();
	return 0;
}
