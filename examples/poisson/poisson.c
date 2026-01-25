/**
 * @file poisson.c
 * @brief Example for poisson equation. Translated from Devito example code.
 *
 * @copyright This software is part of the EPSILOD project by Trasgo Group, UVa.
 * The relevant license, warranty and copyright notice is available in the EPSILOD project repository.
 */

#include <stdio.h>
#include <stdlib.h>

#include <poisson_types.h>
#include <poisson_ext_type.h>
#include <epsilod.h>

char *input_file_name;
char *output_file_name;

vec2l size;
int   iterations;

/* A. INITIALIZE ARRAY */
void initData(HitTile(EPSILOD_BASE_TYPE) tileMat, EpsilodCoords global, Epsilod_ext *ext_params) {
	HitTile root = *hit_tileRoot(&tileMat);
	HitInd  i, j;

	/* 2.1. FIRST COLUMN IS MINE */
	if (hit_sigIn(hit_tileDimSig(tileMat, 1), hit_tileDimBegin(root, 1)))
		for (j = 0; j < global.borders.low[1]; j++)
			hit_tileForDimDomain(tileMat, 0, i)
				hit_tileElemAt(tileMat, 2, i, j) = 0; // 3;

	/* 2.2. LAST COLUMN IS MINE */
	if (hit_sigIn(hit_tileDimSig(tileMat, 1), hit_tileDimEnd(root, 1)))
		for (j = 0; j < global.borders.high[1]; j++)
			hit_tileForDimDomain(tileMat, 0, i)
				hit_tileElemAt(tileMat, 2, i, hit_tileDimCard(tileMat, 1) - 1 - j) = (EPSILOD_BASE_TYPE)i / (size.x - 1); // sqrtf((float)i / size.y); // 4;

	/* 2.3. FIRST ROW IS MINE */
	if (hit_sigIn(hit_tileDimSig(tileMat, 0), hit_tileDimBegin(root, 0)))
		for (i = 0; i < global.borders.low[0]; i++)
			hit_tileForDimDomain(tileMat, 1, j)
				hit_tileElemAt(tileMat, 2, i, j) = 0; // 1;

	/* 2.4. LAST ROW IS MINE */
	if (hit_sigIn(hit_tileDimSig(tileMat, 0), hit_tileDimEnd(root, 0)))
		for (i = 0; i < global.borders.high[0]; i++) {
			hit_tileForDimDomain(tileMat, 1, j)
				hit_tileElemAt(tileMat, 2, hit_tileDimCard(tileMat, 0) - 1 - i, j) = (EPSILOD_BASE_TYPE)j / (size.y - 1); // 2;
		}
}

/* B. WRITE RESULTS */
void outputData(HitTile(EPSILOD_BASE_TYPE) io_tile, Epsilod_ext *ext_params) {

	if (strcmp(output_file_name, "-") == 0) {
		printf("Example not writing output.\n");
		fflush(stdout);
		return;
	}

	char o_f_name[1024];
	sprintf(o_f_name, "%s_%ld_%ld_%d", output_file_name, size.x, size.y, iterations);
	output_file_name = o_f_name;

	/* Write distributed file */
	// var, fileNamePrefix, fileNameSuffix, fileRank, format, coord, header, datatype, formatSize1, formatSize2
	// hit_tileFileWriteOptions(&io_tile, output_file_name, "", HIT_FILE_RUNTIME, HIT_FILE_TEXT, HIT_FILE_ARRAY, HIT_FILE_NO_HEADER, HIT_FILE_FLOAT, 12, 8);
	hit_tileFileWriteOptions(&io_tile, output_file_name, NULL, HIT_FILE_RUNTIME, HIT_FILE_BINARY, HIT_FILE_ARRAY, HIT_FILE_NO_HEADER, HIT_FILE_TYPE_UNKNOWN, 1, 0);
}

/* D. DECLARATIONS OF OPTIMIZED STENCIL KERNEL
 * SEE poisson_kernel.c FILE */
REGISTER_STENCIL(updateCell_poisson, GENERIC, DEFAULT);
REGISTER_STENCIL(noop_poisson, GENERIC, DEFAULT);

/* HELP: PRINT ARGUMENT USAGE */
void printUsage(char *argv[]) {
	if (hit_Rank == 0) {
		fprintf(stderr, "\n=== EPSILOD EXAMPLE: Poisson ===\n");
		fprintf(stderr, "\nUsage: %s <size_x> <size_y> <numIterations> <input_file> <output_file> <device_selection_file>\n", argv[0]);
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
	iterations                  = atoi(argv[3]);
	input_file_name             = argv[4];
	output_file_name            = argv[5];
	char *device_selection_file = argv[6];

	/* STENCIL DECLARATION */
	/* RADIUS */
	int      radius              = 1;
	HitShape shp_stencil_poisson = hitShape((-radius, radius), (-radius, radius));

	/* WEIGHTS: SPECIALIZED KERNEL, VALUES ARE USED ONLY TO COMPUTE BORDERS */
	float stencilData_poisson[] = {
		0, 1, 0,
		1, 0, 1,
		0, 1, 0};

	/* POINTER TO SPECIFIC KERNEL */
	stencilDeviceFunction f_stencil = iterations == 0 ? noop_poisson : updateCell_poisson;

	// int sizes[3] = {size.x + (radius * 2), size.y + (radius * 2), 0};
	HitInd sizes[3] = {size.x, size.y, 0};

	EPSILOD_BASE_TYPE xmin = 0.;
	EPSILOD_BASE_TYPE ymin = 0.;
	EPSILOD_BASE_TYPE xmax = 2.;
	EPSILOD_BASE_TYPE ymax = 1.;
	/* LAUNCH STENCIL COMPUTATION */
	Epsilod_ext ext_params = {
		0,
		(xmax - xmin) / (size.x - 1),
		(ymax - ymin) / (size.y - 1),
	};
	stencilComputation(sizes, shp_stencil_poisson, stencilData_poisson, 1.0f, iterations, initData, NULL, NULL, f_stencil, outputData, &ext_params, device_selection_file);

	/* END */
	Ctrl_Finalize();
	return 0;
}
