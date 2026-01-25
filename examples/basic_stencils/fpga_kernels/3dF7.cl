/**
 * @file basic_stencils/fpga_kernels/3dF7.cl
 * @brief Epsilod: Example with several key stencils. FPGA 3dF7 kernel code.
 *
 * @copyright This software is part of the EPSILOD project by Trasgo Group, UVa.
 * The relevant license, warranty and copyright notice is available in the EPSILOD project repository.
 */

#include "epsilod_types.h"

#include "Ctrl_FPGA_Kernel.h"

Ctrl_NewType(float);

// 3D non-compact, non-symmetric, radius 2: 7-point star forward-down-below with one corner element
__CTRL_FPGA_KERNEL_OPTIMIZE_FOR(8, 64, 4, 1)
__CTRL_FPGA_KERNEL_REPLICATE(2, 2, 2)
CTRL_KERNEL_FN(updateCell_3dF7, FPGA, NDRANGE, KHitTileR_arg(float, matrix), KHitTileR_arg(float, matrixCopy),
			   K_arg(EpsilodCoords, global_coords), KHitTileR_arg(float, stencil), K_arg(float, factor),
			   K_arg(Epsilod_ext, ext_params)) {
	hit(matrix, thr_i, thr_j, thr_k) = (2.0f * (hit(matrixCopy, thr_i - 1, thr_j, thr_k) +
												hit(matrixCopy, thr_i, thr_j - 1, thr_k) +
												hit(matrixCopy, thr_i, thr_j, thr_k - 1)) +
										hit(matrixCopy, thr_i - 2, thr_j, thr_k) +
										hit(matrixCopy, thr_i, thr_j - 2, thr_k) +
										hit(matrixCopy, thr_i, thr_j, thr_k - 2) +
										.5f * hit(matrixCopy, thr_i - 1, thr_j - 1, thr_k - 1)) /
									   9.5f;

	CTRL_KERNEL_END();
}

__CTRL_FPGA_KERNEL_OPTIMIZE_FOR(8, 64, 1, 4)
__CTRL_FPGA_KERNEL_REPLICATE(2, 1, 2)
CTRL_KERNEL_FN(updateCell_3dF7_yBorder, FPGA, NDRANGE, KHitTileR_arg(float, matrix), KHitTileR_arg(float, matrixCopy),
			   K_arg(EpsilodCoords, global_coords), KHitTileR_arg(float, stencil), K_arg(float, factor),
			   K_arg(Epsilod_ext, ext_params)) {
	hit(matrix, thr_i, thr_j, thr_k) = (2.0f * (hit(matrixCopy, thr_i - 1, thr_j, thr_k) +
												hit(matrixCopy, thr_i, thr_j - 1, thr_k) +
												hit(matrixCopy, thr_i, thr_j, thr_k - 1)) +
										hit(matrixCopy, thr_i - 2, thr_j, thr_k) +
										hit(matrixCopy, thr_i, thr_j - 2, thr_k) +
										hit(matrixCopy, thr_i, thr_j, thr_k - 2) +
										.5f * hit(matrixCopy, thr_i - 1, thr_j - 1, thr_k - 1)) /
									   9.5f;

	CTRL_KERNEL_END();
}

__CTRL_FPGA_KERNEL_OPTIMIZE_FOR(8, 1, 64, 4)
__CTRL_FPGA_KERNEL_REPLICATE(1, 2, 2)
CTRL_KERNEL_FN(updateCell_3dF7_xBorder, FPGA, NDRANGE, KHitTileR_arg(float, matrix), KHitTileR_arg(float, matrixCopy),
			   K_arg(EpsilodCoords, global_coords), KHitTileR_arg(float, stencil), K_arg(float, factor),
			   K_arg(Epsilod_ext, ext_params)) {
	hit(matrix, thr_i, thr_j, thr_k) = (2.0f * (hit(matrixCopy, thr_i - 1, thr_j, thr_k) +
												hit(matrixCopy, thr_i, thr_j - 1, thr_k) +
												hit(matrixCopy, thr_i, thr_j, thr_k - 1)) +
										hit(matrixCopy, thr_i - 2, thr_j, thr_k) +
										hit(matrixCopy, thr_i, thr_j - 2, thr_k) +
										hit(matrixCopy, thr_i, thr_j, thr_k - 2) +
										.5f * hit(matrixCopy, thr_i - 1, thr_j - 1, thr_k - 1)) /
									   9.5f;

	CTRL_KERNEL_END();
}

__CTRL_FPGA_KERNEL_OPTIMIZE_FOR(8, 64, 4, 1)
__CTRL_FPGA_KERNEL_REPLICATE(2, 2, 1)
CTRL_KERNEL_FN(updateCell_3dF7_zBorder, FPGA, NDRANGE, KHitTileR_arg(float, matrix), KHitTileR_arg(float, matrixCopy),
			   K_arg(EpsilodCoords, global_coords), KHitTileR_arg(float, stencil), K_arg(float, factor),
			   K_arg(Epsilod_ext, ext_params)) {
	hit(matrix, thr_i, thr_j, thr_k) = (2.0f * (hit(matrixCopy, thr_i - 1, thr_j, thr_k) +
												hit(matrixCopy, thr_i, thr_j - 1, thr_k) +
												hit(matrixCopy, thr_i, thr_j, thr_k - 1)) +
										hit(matrixCopy, thr_i - 2, thr_j, thr_k) +
										hit(matrixCopy, thr_i, thr_j - 2, thr_k) +
										hit(matrixCopy, thr_i, thr_j, thr_k - 2) +
										.5f * hit(matrixCopy, thr_i - 1, thr_j - 1, thr_k - 1)) /
									   9.5f;

	CTRL_KERNEL_END();
}
