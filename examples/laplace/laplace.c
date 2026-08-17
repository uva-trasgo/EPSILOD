/**
 * @file laplace.c
 * @brief Example for laplace equation.
 *
 * @copyright This software is part of the EPSILOD project by Trasgo Group, UVa.
 * The relevant license, warranty and copyright notice is available in the EPSILOD project repository.
 */
#include <stdio.h>
#include <stdlib.h>

#include <laplace_ext_type.h>
#include <epsilod.h>

/* DECLARATIONS OF OPTIMIZED STENCIL KERNEL
 * SEE laplace_kernel.c FILE */
REGISTER_STENCIL(updateCell_laplace, GENERIC, DEFAULT);
REGISTER_INIT(initCell_3D, GENERIC, DEFAULT);

/* HELP: PRINT ARGUMENT USAGE */
void printUsage(char *argv[]) {
	if (hit_Rank == 0) {
		fprintf(stderr, "\n=== EPSILOD EXAMPLE: Laplace ===\n");
		fprintf(stderr, "\nUsage: %s <size_i> <size_j> <size_k> <numIterations> <device_selection_file>\n", argv[0]);
		fprintf(stderr, "\n");
	}
}

/* MAIN: STENCIL PROGRAM, READ ARGUMENTS AND CALL THE PATTERN */
int main(int argc, char *argv[]) {
	/* Init communication system */
	Ctrl_Init(&argc, &argv);

	/* Check program arguments number */
	if (argc != 6) {
		printUsage(argv);
		exit(EXIT_FAILURE);
	}

	HitInd sizes[3] = {};

	/* READ ARGUMENTS */
	sizes[0]                    = atol(argv[1]);
	sizes[1]                    = atol(argv[2]);
	sizes[2]                    = atol(argv[3]);
	int   iterations            = atoi(argv[4]);
	char *device_selection_file = argv[5];

	/* STENCIL DECLARATION */
	int      radius      = 1;
	HitShape stencil_shp = hitShape((-radius, radius), (-radius, radius), (-radius, radius));

	/* WEIGHTS: SPECIALIZED KERNEL, VALUES ARE USED ONLY TO COMPUTE BORDERS */
	float stencil_data[] = {
		0, 0, 0, 0, 1, 0, 0, 0, 0,
		0, 1, 0, 1, 0, 1, 0, 1, 0,
		0, 0, 0, 0, 1, 0, 0, 0, 0};

	/* POINTER TO SPECIFIC KERNEL */
	initDataDeviceFunction f_init    = initCell_3D;
	stencilDeviceFunction  f_stencil = updateCell_laplace;

	/* EXTRA KERNEL PARAMETERS */
	EPSILOD_BASE_TYPE extent_i = 1.;
	EPSILOD_BASE_TYPE extent_j = 1.;
	EPSILOD_BASE_TYPE extent_k = 1.;

	EPSILOD_BASE_TYPE h_i = extent_i / (sizes[0] - 1);
	EPSILOD_BASE_TYPE h_j = extent_j / (sizes[1] - 1);
	EPSILOD_BASE_TYPE h_k = extent_k / (sizes[2] - 1);

	EPSILOD_BASE_TYPE r1 = h_i * h_i;
	EPSILOD_BASE_TYPE r2 = h_j * h_j;
	EPSILOD_BASE_TYPE r3 = h_k * h_k;
	EPSILOD_BASE_TYPE r0 = 1.0F / (r1 * r2 + r1 * r3 + r2 * r3);

	Epsilod_ext ext_params = {r0, r1, r2, r3};

	/* LAUNCH STENCIL COMPUTATION */
	stencilComputation(sizes, stencil_shp, stencil_data, 1.0f, iterations, NULL, f_init, NULL, f_stencil, NULL, &ext_params, device_selection_file);

	/* END */
	Ctrl_Finalize();
	return EXIT_SUCCESS;
}
