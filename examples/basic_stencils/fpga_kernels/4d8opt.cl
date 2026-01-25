/**
 * @file basic_stencils/fpga_kernels/4d8opt.cl
 * @brief Epsilod: Example with several key stencils. Optimized FPGA 4d8 kernel code.
 *
 * @copyright This software is part of the EPSILOD project by Trasgo Group, UVa.
 * The relevant license, warranty and copyright notice is available in the EPSILOD project repository.
 */

#include "epsilod_types.h"
#include "opt_common.h"

#include "Ctrl_FPGA_Kernel.h"

Ctrl_NewType(float);

/* Constant sizes for stencil kernels */

#define RAD 1 // Stencil radius

#define HALO_BLOCK_WIDTH_X (DEFAULT_BLOCK_WIDTH_4D * 2)   // With halos
#define HALO_BLOCK_WIDTH_Y DEFAULT_BLOCK_WIDTH_4D         // With halos
#define HALO_BLOCK_WIDTH_Z DEFAULT_BLOCK_WIDTH_4D         // With halos
#define BLOCK_WIDTH_X      (HALO_BLOCK_WIDTH_X - 2 * RAD) // Without halos
#define BLOCK_WIDTH_Y      (HALO_BLOCK_WIDTH_Y - 2 * RAD) // Without halos
#define BLOCK_WIDTH_Z      (HALO_BLOCK_WIDTH_Y - 2 * RAD) // Without halos

#ifndef VEC_SIZE
#define VEC_SIZE DEFAULT_VEC_SIZE
#endif // VEC_SIZE

// HALO_BLOCK_WIDTH_X must be multiple of VEC_SIZE
#if HALO_BLOCK_WIDTH_X % VEC_SIZE != 0
#error HALO_BLOCK_WIDTH_X is not multiple of VEC_SIZE. This would cause incorrect hardware with out-of-bounds memory accesses.
#endif // HALO_BLOCK_WIDTH_X % VEC_SIZE != 0

// Elements whose input is a posterior shift register element (i.e., does not include last row)
#define SR_BASE      (2 * RAD * HALO_BLOCK_WIDTH_X * HALO_BLOCK_WIDTH_Y * HALO_BLOCK_WIDTH_Z)
#define SR_TAIL_SIZE (VEC_SIZE)               // Elements whose input is a global memory datum
#define SR_SIZE      (SR_BASE + SR_TAIL_SIZE) // Total shift register size
#define SR_CENTER    (SR_BASE / 2)            // Index of the center element of the shift register

#if SR_SIZE % VEC_SIZE != 0
#error SR_SIZE is not multiple of VEC_SIZE. This would cause incorrect hardware with out-of-bounds memory accesses.
#endif // SR_SIZE % VEC_SIZE != 0

// 4D compact, radius 1: 8-point star, no corners
EPSILOD_FPGA_BASIC_STENCIL(4d8, matrix, matrixCopy) {
	int x_size = hit_tileDimCard(matrix, 3); // Elements computed in x dimension
	int y_size = hit_tileDimCard(matrix, 2); // Elements computed in y dimension
	int z_size = hit_tileDimCard(matrix, 1); // Elements computed in z dimension
	int w_size = hit_tileDimCard(matrix, 0); // Elements computed in w dimension

	// Number of spatial blocks that will be computed:
	int blocks_x = (x_size + BLOCK_WIDTH_X - 1) / BLOCK_WIDTH_X;
	int blocks_y = (y_size + BLOCK_WIDTH_Y - 1) / BLOCK_WIDTH_Y;
	int blocks_z = (z_size + BLOCK_WIDTH_Z - 1) / BLOCK_WIDTH_Z;
	int n_blocks = blocks_x * blocks_y;

#include "kernel_body/4d8opt.h"

	CTRL_KERNEL_END();
}

/* Constant sizes for XZW border kernel */
// Basically, the shift register now is smaller/narrower, as the vertical borders are
// very narrow (radius size + halos). Using the inner's kernel's size would be a waste
// of cycles and resources.

#define BLOCK_WIDTH_Y_YBORDER      (RAD)
#define HALO_BLOCK_WIDTH_Y_YBORDER (BLOCK_WIDTH_Y_YBORDER + 2 * RAD)

#define SR_BASE_YBORDER (2 * RAD * HALO_BLOCK_WIDTH_X * HALO_BLOCK_WIDTH_Y_YBORDER * HALO_BLOCK_WIDTH_Z)
#define SR_SIZE_YBORDER (SR_BASE_YBORDER + SR_TAIL_SIZE)

// Reuse previous names:
#undef HALO_BLOCK_WIDTH_Y
#define HALO_BLOCK_WIDTH_Y HALO_BLOCK_WIDTH_Y_YBORDER

#undef BLOCK_WIDTH_Y
#define BLOCK_WIDTH_Y BLOCK_WIDTH_Y_YBORDER

#undef SR_BASE
#define SR_BASE SR_BASE_YBORDER

#undef SR_SIZE
#define SR_SIZE SR_SIZE_YBORDER

EPSILOD_FPGA_BASIC_STENCIL(4d8_yBorder, matrix, matrixCopy) {
	// We know some of these sizes at compile time:
	int x_size = hit_tileDimCard(matrix, 3); // Elements computed in x dimension
	int y_size = BLOCK_WIDTH_Y;              // Elements computed in y dimension
	int z_size = hit_tileDimCard(matrix, 1); // Elements computed in z dimension
	int w_size = hit_tileDimCard(matrix, 0); // Elements computed in w dimension

	// Number of (X-Z) spatial blocks that will be computed:
	int blocks_x = (x_size + BLOCK_WIDTH_X - 1) / BLOCK_WIDTH_X;
	int blocks_z = (z_size + BLOCK_WIDTH_Z - 1) / BLOCK_WIDTH_Z;
	int n_blocks = blocks_x * blocks_z;

#define MORE_THAN_ONE_Y_BLOCK false
#include "kernel_body/4d8opt.h"

	CTRL_KERNEL_END();
}

/* Constant sizes for YZW border kernel */
// Basically, the shift register now is smaller/narrower, as the vertical borders are
// very narrow (radius size + halos). Using the inner's kernel's size would be a waste
// of cycles and resources.

#define BLOCK_WIDTH_X_XBORDER      (RAD)
#define HALO_BLOCK_WIDTH_X_XBORDER (BLOCK_WIDTH_X_XBORDER + 2 * RAD)

#define SR_BASE_XBORDER (2 * RAD * HALO_BLOCK_WIDTH_X_XBORDER * HALO_BLOCK_WIDTH_Y * HALO_BLOCK_WIDTH_Z)
#define SR_SIZE_XBORDER (SR_BASE_XBORDER + SR_TAIL_SIZE)

// Reuse previous names:
#undef HALO_BLOCK_WIDTH_X
#define HALO_BLOCK_WIDTH_X HALO_BLOCK_WIDTH_X_XBORDER

#undef BLOCK_WIDTH_X
#define BLOCK_WIDTH_X BLOCK_WIDTH_X_XBORDER

#undef SR_BASE
#define SR_BASE SR_BASE_XBORDER

#undef SR_SIZE
#define SR_SIZE SR_SIZE_XBORDER

// Recover original values for previously overridden names:
#undef HALO_BLOCK_WIDTH_Y
#define HALO_BLOCK_WIDTH_Y DEFAULT_BLOCK_WIDTH_4D

#undef BLOCK_WIDTH_Y
#define BLOCK_WIDTH_Y (HALO_BLOCK_WIDTH_Y - 2 * RAD)

// HALO_BLOCK_WIDTH_Y must be multiple of VEC_SIZE for this kernel
#if HALO_BLOCK_WIDTH_Y % VEC_SIZE != 0
#error HALO_BLOCK_WIDTH_Y is not multiple of VEC_SIZE. This would cause incorrect hardware with out-of-bounds memory accesses in the YZ border kernel.
#endif // HALO_BLOCK_WIDTH_X % VEC_SIZE != 0

// NOTE: This kernel will advance in the Y dimension instead of in the X dimension, to allow for vectorization
EPSILOD_FPGA_BASIC_STENCIL(4d8_xBorder, matrix, matrixCopy) {
	// We know some of these sizes at compile time:
	int x_size = BLOCK_WIDTH_X;              // Elements computed in x dimension
	int y_size = hit_tileDimCard(matrix, 2); // Elements computed in y dimension
	int z_size = hit_tileDimCard(matrix, 1); // Elements computed in z dimension
	int w_size = hit_tileDimCard(matrix, 0); // Elements computed in w dimension

	// Number of (Y-Z) spatial blocks that will be computed:
	int blocks_y = (y_size + BLOCK_WIDTH_Y - 1) / BLOCK_WIDTH_Y;
	int blocks_z = (z_size + BLOCK_WIDTH_Z - 1) / BLOCK_WIDTH_Z;
	int n_blocks = blocks_y * blocks_z;

// Swap x and y in the global array accesses, to traverse the y dimension first (to enable vectorization):
#define HIT(arr, dim3, dim2, dim0, dim1) hit(arr, dim3, dim2, dim1, dim0)
#define MORE_THAN_ONE_Y_BLOCK            false
#include "kernel_body/4d8opt.h"

	CTRL_KERNEL_END();
}

/* Constant sizes for XYW border kernel */
// Basically, the shift register now is smaller/narrower, as the vertical borders are
// very narrow (radius size + halos). Using the inner's kernel's size would be a waste
// of cycles and resources.

#define BLOCK_WIDTH_Z_ZBORDER      (RAD)
#define HALO_BLOCK_WIDTH_Z_ZBORDER (BLOCK_WIDTH_Z_ZBORDER + 2 * RAD)

#define SR_BASE_ZBORDER (2 * RAD * HALO_BLOCK_WIDTH_X * HALO_BLOCK_WIDTH_Y * HALO_BLOCK_WIDTH_Z_ZBORDER)
#define SR_SIZE_ZBORDER (SR_BASE_ZBORDER + SR_TAIL_SIZE)

// Reuse previous names:
#undef HALO_BLOCK_WIDTH_Z
#define HALO_BLOCK_WIDTH_Z HALO_BLOCK_WIDTH_Z_ZBORDER

#undef BLOCK_WIDTH_Z
#define BLOCK_WIDTH_Z BLOCK_WIDTH_Z_ZBORDER

#undef SR_BASE
#define SR_BASE SR_BASE_ZBORDER

#undef SR_SIZE
#define SR_SIZE SR_SIZE_ZBORDER

// Recover original values for previously overridden names:
#undef HALO_BLOCK_WIDTH_X
#define HALO_BLOCK_WIDTH_X DEFAULT_BLOCK_WIDTH_4D

#undef BLOCK_WIDTH_X
#define BLOCK_WIDTH_X (HALO_BLOCK_WIDTH_X - 2 * RAD)

// NOTE: This kernel will advance in the Y dimension instead of in the X dimension, to allow for vectorization
EPSILOD_FPGA_BASIC_STENCIL(4d8_zBorder, matrix, matrixCopy) {
	// We know some of these sizes at compile time:
	int x_size = hit_tileDimCard(matrix, 3); // Elements computed in x dimension
	int y_size = hit_tileDimCard(matrix, 2); // Elements computed in y dimension
	int z_size = BLOCK_WIDTH_Z;              // Elements computed in z dimension
	int w_size = hit_tileDimCard(matrix, 0); // Elements computed in w dimension

	// Number of (X-Y) spatial blocks that will be computed:
	int blocks_x = (x_size + BLOCK_WIDTH_X - 1) / BLOCK_WIDTH_X;
	int blocks_y = (y_size + BLOCK_WIDTH_Y - 1) / BLOCK_WIDTH_Y;
	int n_blocks = blocks_x * blocks_y;

#define MORE_THAN_ONE_Z_BLOCK false
#include "kernel_body/4d8opt.h"

	CTRL_KERNEL_END();
}
