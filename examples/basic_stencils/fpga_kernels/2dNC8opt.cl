/**
 * @file basic_stencils/fpga_kernels/2dNC8opt.cl
 * @brief Epsilod: Example with several key stencils. Optimized FPGA 2dNC8 kernel code.
 *
 * @copyright This software is part of the EPSILOD project by Trasgo Group, UVa.
 * The relevant license, warranty and copyright notice is available in the EPSILOD project repository.
 */

#include "epsilod_types.h"
#include "opt_common.h"

#include "Ctrl_FPGA_Kernel.h"

Ctrl_NewType(float);

/* Constant sizes for stencil kernels */

#define RAD 2 // Stencil radius

#define HALO_BLOCK_WIDTH DEFAULT_BLOCK_WIDTH_2D       // With halos
#define BLOCK_WIDTH      (HALO_BLOCK_WIDTH - 2 * RAD) // Without halos

#ifndef VEC_SIZE
#define VEC_SIZE DEFAULT_VEC_SIZE
#endif // VEC_SIZE

// HALO_BLOCK_WIDTH must be multiple of VEC_SIZE
#if HALO_BLOCK_WIDTH % VEC_SIZE != 0
#error HALO_BLOCK_WIDTH is not multiple of VEC_SIZE. This would cause incorrect hardware with out-of-bounds memory accesses.
#endif // HALO_BLOCK_WIDTH % VEC_SIZE != 0

#define SR_BASE (2 * RAD * HALO_BLOCK_WIDTH) // Elements whose input is a posterior shift
// SR dims:      yyyyyyy   xxxxxxxxxxxxxxxx     register element
#define SR_TAIL_SIZE (VEC_SIZE)               // Elements whose input is a global memory datum
#define SR_SIZE      (SR_BASE + SR_TAIL_SIZE) // Total shift register size
#define SR_CENTER    (SR_BASE / 2)            // Index of the center element of the shift register

#if SR_SIZE % VEC_SIZE != 0
#error SR_SIZE is not multiple of VEC_SIZE. This would cause incorrect hardware with out-of-bounds memory accesses.
#endif // SR_SIZE % VEC_SIZE != 0

// 2D non-compact, radius 2: 8-point star, no corners
EPSILOD_FPGA_BASIC_STENCIL(2dNC8, matrix, matrixCopy) {
	int x_size = hit_tileDimCard(matrix, 1); // Elements computed in x dimension
	int y_size = hit_tileDimCard(matrix, 0); // Elements computed in y dimension

	// Number of spatial blocks that will be computed:
	int n_blocks = (x_size + BLOCK_WIDTH - 1) / BLOCK_WIDTH;

#include "kernel_body/2dNC8opt.h"

	#ifdef PRINT_MATRIX
	if (y_size > 3 * RAD)
		for (int y = -2 * RAD; y < y_size + 2 * RAD; y++) {
			for (int x = -2 * RAD; x < x_size + 2 * RAD; x++)
				printf("%g ", hit(matrix, y, x));
			printf("\n");
		}
	printf("\n");
	#endif // PRINT_MATRIX

	CTRL_KERNEL_END();
}

/* Constant sizes for vertical border kernel */
// Basically, the shift register now is smaller/narrower, as the vertical borders are
// very narrow (radius size + halos). Using the inner's kernel's size would be a waste
// of cycles and resources.

#define BLOCK_WIDTH_XBORDER      (RAD)
#define HALO_BLOCK_WIDTH_XBORDER (BLOCK_WIDTH_XBORDER + 2 * RAD)

#undef VEC_SIZE
#define VEC_SIZE HALO_BLOCK_WIDTH_XBORDER

#define SR_BASE_XBORDER (2 * RAD * HALO_BLOCK_WIDTH_XBORDER)
#define SR_SIZE_XBORDER (SR_BASE_XBORDER + SR_TAIL_SIZE)

// Reuse previous names:
#undef HALO_BLOCK_WIDTH
#define HALO_BLOCK_WIDTH HALO_BLOCK_WIDTH_XBORDER

#undef BLOCK_WIDTH
#define BLOCK_WIDTH BLOCK_WIDTH_XBORDER

#undef SR_BASE
#define SR_BASE SR_BASE_XBORDER

#undef SR_SIZE
#define SR_SIZE SR_SIZE_XBORDER

EPSILOD_FPGA_BASIC_STENCIL(2dNC8_xBorder, matrix, matrixCopy) {
	// We know some of these sizes at compile time:
	int x_size = BLOCK_WIDTH;                // Elements computed in x dimension
	int y_size = hit_tileDimCard(matrix, 0); // Elements computed in y dimension

	// Number of spatial blocks that will be computed:
	int n_blocks = 1; // Only one block for borders

#define MORE_THAN_ONE_BLOCK false
#include "kernel_body/2dNC8opt.h"

	CTRL_KERNEL_END();
}
