/**
 * @file basic_stencils/fpga_kernels/2d8.cl
 * @brief Epsilod: Example with several key stencils. FPGA 2d8 kernel code.
 *
 * @copyright This software is part of the EPSILOD project by Trasgo Group, UVa.
 * The relevant license, warranty and copyright notice is available in the EPSILOD project repository.
 */

#include "epsilod_types.h"

#include "Ctrl_FPGA_Kernel.h"

Ctrl_NewType(float);

// 2D compact, radius 1: 8-point star, corners included
__CTRL_FPGA_KERNEL_OPTIMIZE_FOR(8, 64, 4, 1)
__CTRL_FPGA_KERNEL_REPLICATE(2, 2)
CTRL_KERNEL_FN(updateCell_2d8, FPGA, NDRANGE, KHitTileR_arg(float, matrix), KHitTileR_arg(float, matrixCopy),
			   K_arg(EpsilodCoords, global_coords), KHitTileR_arg(float, stencil), K_arg(float, factor),
			   K_arg(Epsilod_ext, ext_params)) {
	hit(matrix, thr_i, thr_j) = (4 * (hit(matrixCopy, thr_i - 1, thr_j) + hit(matrixCopy, thr_i + 1, thr_j) + hit(matrixCopy, thr_i, thr_j - 1) + hit(matrixCopy, thr_i, thr_j + 1)) +
								 (hit(matrixCopy, thr_i - 1, thr_j - 1) + hit(matrixCopy, thr_i + 1, thr_j - 1) + hit(matrixCopy, thr_i - 1, thr_j + 1) + hit(matrixCopy, thr_i + 1, thr_j + 1))) /
								20;

	CTRL_KERNEL_END();
}

__CTRL_FPGA_KERNEL_OPTIMIZE_FOR(8, 1, 256, 1)
__CTRL_FPGA_KERNEL_REPLICATE(1, 4)
CTRL_KERNEL_FN(updateCell_2d8_xBorder, FPGA, NDRANGE, KHitTileR_arg(float, matrix), KHitTileR_arg(float, matrixCopy),
			   K_arg(EpsilodCoords, global_coords), KHitTileR_arg(float, stencil), K_arg(float, factor),
			   K_arg(Epsilod_ext, ext_params)) {
	hit(matrix, thr_i, thr_j) = (4 * (hit(matrixCopy, thr_i - 1, thr_j) + hit(matrixCopy, thr_i + 1, thr_j) + hit(matrixCopy, thr_i, thr_j - 1) + hit(matrixCopy, thr_i, thr_j + 1)) +
								 (hit(matrixCopy, thr_i - 1, thr_j - 1) + hit(matrixCopy, thr_i + 1, thr_j - 1) + hit(matrixCopy, thr_i - 1, thr_j + 1) + hit(matrixCopy, thr_i + 1, thr_j + 1))) /
								20;

	CTRL_KERNEL_END();
}

__CTRL_FPGA_KERNEL_OPTIMIZE_FOR(1, 256, 1, 1)
__CTRL_FPGA_KERNEL_REPLICATE(4)
CTRL_KERNEL_FN(updateCell_2d8_yBorder, FPGA, NDRANGE, KHitTileR_arg(float, matrix), KHitTileR_arg(float, matrixCopy),
			   K_arg(EpsilodCoords, global_coords), KHitTileR_arg(float, stencil), K_arg(float, factor),
			   K_arg(Epsilod_ext, ext_params)) {
	hit(matrix, thr_i, thr_j) = (4 * (hit(matrixCopy, thr_i - 1, thr_j) + hit(matrixCopy, thr_i + 1, thr_j) + hit(matrixCopy, thr_i, thr_j - 1) + hit(matrixCopy, thr_i, thr_j + 1)) +
								 (hit(matrixCopy, thr_i - 1, thr_j - 1) + hit(matrixCopy, thr_i + 1, thr_j - 1) + hit(matrixCopy, thr_i - 1, thr_j + 1) + hit(matrixCopy, thr_i + 1, thr_j + 1))) /
								20;

	CTRL_KERNEL_END();
}
