/**
 * @file basic_stencils/fpga_kernels/2dF5.cl
 * @brief Epsilod: Example with several key stencils. FPGA 2dF5 kernel code.
 *
 * @copyright This software is part of the EPSILOD project by Trasgo Group, UVa.
 * The relevant license, warranty and copyright notice is available in the EPSILOD project repository.
 */

#include "epsilod_types.h"

#include "Ctrl_FPGA_Kernel.h"

Ctrl_NewType(float);

// 2D non-compact, non-symmetric, radius 2: 5-point star forward-down with one corner element
__CTRL_FPGA_KERNEL_OPTIMIZE_FOR(8, 64, 4, 1)
__CTRL_FPGA_KERNEL_REPLICATE(2, 2)
CTRL_KERNEL_FN(updateCell_2dF5, FPGA, NDRANGE, KHitTileR_arg(float, matrix), KHitTileR_arg(float, matrixCopy),
			   K_arg(EpsilodCoords, global_coords), KHitTileR_arg(float, stencil), K_arg(float, factor),
			   K_arg(Epsilod_ext, ext_params)) {
	hit(matrix, thr_i, thr_j) = (2.0f * (hit(matrixCopy, thr_i - 1, thr_j) + hit(matrixCopy, thr_i, thr_j - 1)) +
								 (hit(matrixCopy, thr_i - 2, thr_j) + hit(matrixCopy, thr_i, thr_j - 2)) +
								 .5f * hit(matrixCopy, thr_i - 1, thr_j - 1)) /
								6.5f;

	CTRL_KERNEL_END();
}

__CTRL_FPGA_KERNEL_OPTIMIZE_FOR(8, 1, 256, 1)
__CTRL_FPGA_KERNEL_REPLICATE(1, 4)
CTRL_KERNEL_FN(updateCell_2dF5_xBorder, FPGA, NDRANGE, KHitTileR_arg(float, matrix), KHitTileR_arg(float, matrixCopy),
			   K_arg(EpsilodCoords, global_coords), KHitTileR_arg(float, stencil), K_arg(float, factor),
			   K_arg(Epsilod_ext, ext_params)) {
	hit(matrix, thr_i, thr_j) = (2.0f * (hit(matrixCopy, thr_i - 1, thr_j) + hit(matrixCopy, thr_i, thr_j - 1)) +
								 (hit(matrixCopy, thr_i - 2, thr_j) + hit(matrixCopy, thr_i, thr_j - 2)) +
								 .5f * hit(matrixCopy, thr_i - 1, thr_j - 1)) /
								6.5f;

	CTRL_KERNEL_END();
}

__CTRL_FPGA_KERNEL_OPTIMIZE_FOR(8, 256, 1, 1)
__CTRL_FPGA_KERNEL_REPLICATE(4)
CTRL_KERNEL_FN(updateCell_2dF5_yBorder, FPGA, NDRANGE, KHitTileR_arg(float, matrix), KHitTileR_arg(float, matrixCopy),
			   K_arg(EpsilodCoords, global_coords), KHitTileR_arg(float, stencil), K_arg(float, factor),
			   K_arg(Epsilod_ext, ext_params)) {
	hit(matrix, thr_i, thr_j) = (2.0f * (hit(matrixCopy, thr_i - 1, thr_j) + hit(matrixCopy, thr_i, thr_j - 1)) +
								 (hit(matrixCopy, thr_i - 2, thr_j) + hit(matrixCopy, thr_i, thr_j - 2)) +
								 .5f * hit(matrixCopy, thr_i - 1, thr_j - 1)) /
								6.5f;

	CTRL_KERNEL_END();
}
