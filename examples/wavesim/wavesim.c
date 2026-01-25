/**
 * @file wavesim.c
 * @brief Example for wave simulation. Translated from Celerity example code.
 *
 * @copyright This software is part of the EPSILOD project by Trasgo Group, UVa.
 * The relevant license, warranty and copyright notice is available in the EPSILOD project repository.
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "wavesim_types.h"
#include "wavesim_ext_type.h"
#include "epsilod.h"

char *output_file_name;

vec2l size;
int   iterations;

void initData(HitTile(EPSILOD_BASE_TYPE) tileMat, EpsilodCoords global_coords, Epsilod_ext *p_ext_params) {
	Epsilod_ext ext_params = *p_ext_params;

	int i, j;

	const vec2f *const      s = &ext_params.sigma;
	const EPSILOD_BASE_TYPE a = ext_params.amplitude;

	int radius = 1;

	hit_tileForDimDomain(tileMat, 0, i) {
		hit_tileForDimDomain(tileMat, 1, j) {
			const int x_g = j + global_coords.offset[1] - radius;
			const int y_g = i + global_coords.offset[0] - radius;

			const float dx = x_g - ext_params.center.x;
			const float dy = y_g - ext_params.center.y;

			hit(tileMat, i, j) = a * expf(-(dx * dx / (2.f * s->x * s->x) + dy * dy / (2.f * s->y * s->y)));
		}
	}
}

/* WRITE RESULTS */
void outputData(HitTile_float io_tile, Epsilod_ext *ext_params) {
	if (strcmp(output_file_name, "-") == 0) {
		return;
	}

	char o_f_name[1024];
	sprintf(o_f_name, "%s_%ld_%ld_%d", output_file_name, size.x, size.y, iterations);
	output_file_name = o_f_name;

	int radius = 1;

	/* DATA PART */
	/* Create a virtual tile with the real shape to use as reference */
	HitShape shp_root = hitShape(
		(radius, size.x + radius - 1),
		(radius, size.y + radius - 1));
	HitTile(EPSILOD_BASE_TYPE) virtual_root;
	hit_tileDomainShape(&virtual_root, EPSILOD_BASE_TYPE, shp_root);
	io_tile.ref = &virtual_root;

	/* Select the local data part without borders */
	HitShape shp_data = hitShape(
		(radius, size.x + radius - 1),
		(radius, size.y + radius - 1));
	HitShape shp_local_data = hit_shapeIntersect(shp_data, hit_tileShape(io_tile));
	HitTile(EPSILOD_BASE_TYPE) data;
	hit_tileSelectArrayCoords(&data, &io_tile, shp_local_data);

	/* Write distributed file */
	// var, fileNamePrefix, fileNameSuffix, fileRank, format, coord, header, datatype, formatSize1, formatSize2
	hit_tileFileWriteOptions(&data, output_file_name, NULL, HIT_FILE_RUNTIME, HIT_FILE_BINARY, HIT_FILE_ARRAY, HIT_FILE_NO_HEADER, HIT_FILE_TYPE_UNKNOWN, 1, 0);
}

/* DECLARATIONS OF OPTIMIZED STENCIL KERNEL
 * SEE wavesim_kernels.c FILE */
REGISTER_STENCIL(updateCell_wavesim, GENERIC, DEFAULT);
REGISTER_STENCIL(initCellCopy_wavesim, GENERIC, DEFAULT);
REGISTER_INIT(initCell_wavesim, GENERIC, DEFAULT);

/* HELP: PRINT ARGUMENT USAGE */
void printUsage(char *argv[]) {
	if (hit_Rank == 0) {
		fprintf(stderr, "\n=== EPSILOD EXAMPLE: Wave Simulation ===\n");
		fprintf(stderr, "\nUsage: %s <size_x> <size_y> <T> <dt> <output_file> <device_selection_file>\n", argv[0]);
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
	size.y                      = atol(argv[1]);
	size.x                      = atol(argv[2]);
	int   T                     = atoi(argv[3]);
	float dt                    = atof(argv[4]);
	iterations                  = T / dt;
	output_file_name            = argv[5];
	char *device_selection_file = argv[6];

	/* STENCIL DECLARATION */
	/* RADIUS */
	int      radius      = 1;
	HitShape shp_stencil = hitShape((-radius, radius), (-radius, radius));

	/* WEIGHTS: SPECIALIZED KERNEL, VALUES ARE USED ONLY TO COMPUTE BORDERS */
	float stencilData[] = {
		0, 1, 0,
		1, 1, 1,
		0, 1, 0};

	/* POINTER TO SPECIFIC KERNEL */
	stencilDeviceFunction  f_stencil   = updateCell_wavesim;
	stencilDeviceFunction  f_init_copy = initCellCopy_wavesim;
	initDataDeviceFunction f_init      = initCell_wavesim;

	HitInd sizes[3] = {size.y + (radius * 2), size.x + (radius * 2), 0};

	/* LAUNCH STENCIL COMPUTATION */
	Epsilod_ext ext_params;
	ext_params.dt        = dt;
	ext_params.dx        = 1.f;
	ext_params.dy        = 1.f;
	ext_params.center    = (vec2f){size.x / 4.f, size.y / 4.f};
	ext_params.amplitude = 1.f;
	ext_params.sigma     = (vec2f){size.x / 8.f, size.y / 8.f};
	stencilComputation(sizes, shp_stencil, stencilData, 1.0f, iterations, NULL, f_init, f_init_copy, f_stencil, outputData, &ext_params, device_selection_file);

	/* END */
	Ctrl_Finalize();
	return 0;
}
