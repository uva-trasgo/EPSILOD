/**
 * @file basic_stencils/fpga_kernels/opt_common.h
 * @brief Epsilod: Example with several key stencils. Default constants for FPGA-optimized kernels.
 *
 * @copyright This software is part of the EPSILOD project by Trasgo Group, UVa.
 * The relevant license, warranty and copyright notice is available in the EPSILOD project repository.
 */

#ifndef _EPSILOD_FPGA_KERNELS_COMMON_H_
#define _EPSILOD_FPGA_KERNELS_COMMON_H_

#define DEFAULT_BLOCK_WIDTH    4096
#define DEFAULT_BLOCK_WIDTH_2D DEFAULT_BLOCK_WIDTH
#define DEFAULT_BLOCK_WIDTH_3D (DEFAULT_BLOCK_WIDTH / 16)
#define DEFAULT_BLOCK_WIDTH_4D (DEFAULT_BLOCK_WIDTH / 128)
#define DEFAULT_VEC_SIZE       16

#define EPSILOD_FPGA_BASIC_STENCIL(stencil, matrix, matrixCopy)                                             \
	CTRL_KERNEL_FN(updateCell_##stencil, FPGA, TASK,                                                        \
				   KHitTileR_arg(float, matrix), KHitTileR_arg(float, matrixCopy),                          \
				   K_arg(EpsilodCoords, _global_coords), KHitTileR_arg(float, _pat), K_arg(float, _factor), \
				   K_arg(Epsilod_ext, _ext_params))

#endif // !_EPSILOD_FPGA_KERNELS_COMMON_H_
