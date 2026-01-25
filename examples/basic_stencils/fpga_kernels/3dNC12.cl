/**
 * @file basic_stencils/fpga_kernels/3dNC12.cl
 * @brief Epsilod: Example with several key stencils. FPGA 3dNC12 kernel code.
 *
 * @copyright This software is part of the EPSILOD project by Trasgo Group, UVa.
 * The relevant license, warranty and copyright notice is available in the EPSILOD project repository.
 */

#include "epsilod_types.h"

#include "Ctrl_FPGA_Kernel.h"

Ctrl_NewType(float);

// 3D non-compact, radius 2: 12-point star, no corners
__CTRL_FPGA_KERNEL_OPTIMIZE_FOR(8, 64, 4, 1)
__CTRL_FPGA_KERNEL_REPLICATE(2, 2, 2)
CTRL_KERNEL_FN(updateCell_3dNC12, FPGA, NDRANGE, KHitTileR_arg(float, matrix), KHitTileR_arg(float, matrixCopy),
			   K_arg(EpsilodCoords, global_coords), KHitTileR_arg(float, stencil), K_arg(float, factor),
			   K_arg(Epsilod_ext, ext_params)) {
	hit(matrix, thr_i, thr_j, thr_k) = (hit(matrixCopy, thr_i - 2, thr_j, thr_k) +
										hit(matrixCopy, thr_i + 2, thr_j, thr_k) +
										hit(matrixCopy, thr_i, thr_j - 2, thr_k) +
										hit(matrixCopy, thr_i, thr_j + 2, thr_k) +
										hit(matrixCopy, thr_i, thr_j, thr_k - 2) +
										hit(matrixCopy, thr_i, thr_j, thr_k + 2) +
										6 * (hit(matrixCopy, thr_i - 1, thr_j, thr_k) +
											 hit(matrixCopy, thr_i + 1, thr_j, thr_k) +
											 hit(matrixCopy, thr_i, thr_j - 1, thr_k) +
											 hit(matrixCopy, thr_i, thr_j + 1, thr_k) +
											 hit(matrixCopy, thr_i, thr_j, thr_k - 1) +
											 hit(matrixCopy, thr_i, thr_j, thr_k + 1))) /
									   42;

	CTRL_KERNEL_END();
}

__CTRL_FPGA_KERNEL_OPTIMIZE_FOR(8, 64, 1, 4)
__CTRL_FPGA_KERNEL_REPLICATE(2, 1, 2)
CTRL_KERNEL_FN(updateCell_3dNC12_yBorder, FPGA, NDRANGE, KHitTileR_arg(float, matrix), KHitTileR_arg(float, matrixCopy),
			   K_arg(EpsilodCoords, global_coords), KHitTileR_arg(float, stencil), K_arg(float, factor),
			   K_arg(Epsilod_ext, ext_params)) {
	hit(matrix, thr_i, thr_j, thr_k) = (hit(matrixCopy, thr_i - 2, thr_j, thr_k) +
										hit(matrixCopy, thr_i + 2, thr_j, thr_k) +
										hit(matrixCopy, thr_i, thr_j - 2, thr_k) +
										hit(matrixCopy, thr_i, thr_j + 2, thr_k) +
										hit(matrixCopy, thr_i, thr_j, thr_k - 2) +
										hit(matrixCopy, thr_i, thr_j, thr_k + 2) +
										6 * (hit(matrixCopy, thr_i - 1, thr_j, thr_k) +
											 hit(matrixCopy, thr_i + 1, thr_j, thr_k) +
											 hit(matrixCopy, thr_i, thr_j - 1, thr_k) +
											 hit(matrixCopy, thr_i, thr_j + 1, thr_k) +
											 hit(matrixCopy, thr_i, thr_j, thr_k - 1) +
											 hit(matrixCopy, thr_i, thr_j, thr_k + 1))) /
									   42;

	CTRL_KERNEL_END();
}

__CTRL_FPGA_KERNEL_OPTIMIZE_FOR(8, 1, 64, 4)
__CTRL_FPGA_KERNEL_REPLICATE(1, 2, 2)
CTRL_KERNEL_FN(updateCell_3dNC12_xBorder, FPGA, NDRANGE, KHitTileR_arg(float, matrix), KHitTileR_arg(float, matrixCopy),
			   K_arg(EpsilodCoords, global_coords), KHitTileR_arg(float, stencil), K_arg(float, factor),
			   K_arg(Epsilod_ext, ext_params)) {
	hit(matrix, thr_i, thr_j, thr_k) = (hit(matrixCopy, thr_i - 2, thr_j, thr_k) +
										hit(matrixCopy, thr_i + 2, thr_j, thr_k) +
										hit(matrixCopy, thr_i, thr_j - 2, thr_k) +
										hit(matrixCopy, thr_i, thr_j + 2, thr_k) +
										hit(matrixCopy, thr_i, thr_j, thr_k - 2) +
										hit(matrixCopy, thr_i, thr_j, thr_k + 2) +
										6 * (hit(matrixCopy, thr_i - 1, thr_j, thr_k) +
											 hit(matrixCopy, thr_i + 1, thr_j, thr_k) +
											 hit(matrixCopy, thr_i, thr_j - 1, thr_k) +
											 hit(matrixCopy, thr_i, thr_j + 1, thr_k) +
											 hit(matrixCopy, thr_i, thr_j, thr_k - 1) +
											 hit(matrixCopy, thr_i, thr_j, thr_k + 1))) /
									   42;

	CTRL_KERNEL_END();
}

__CTRL_FPGA_KERNEL_OPTIMIZE_FOR(8, 64, 4, 1)
__CTRL_FPGA_KERNEL_REPLICATE(2, 2, 1)
CTRL_KERNEL_FN(updateCell_3dNC12_zBorder, FPGA, NDRANGE, KHitTileR_arg(float, matrix), KHitTileR_arg(float, matrixCopy),
			   K_arg(EpsilodCoords, global_coords), KHitTileR_arg(float, stencil), K_arg(float, factor),
			   K_arg(Epsilod_ext, ext_params)) {
	hit(matrix, thr_i, thr_j, thr_k) = (hit(matrixCopy, thr_i - 2, thr_j, thr_k) +
										hit(matrixCopy, thr_i + 2, thr_j, thr_k) +
										hit(matrixCopy, thr_i, thr_j - 2, thr_k) +
										hit(matrixCopy, thr_i, thr_j + 2, thr_k) +
										hit(matrixCopy, thr_i, thr_j, thr_k - 2) +
										hit(matrixCopy, thr_i, thr_j, thr_k + 2) +
										6 * (hit(matrixCopy, thr_i - 1, thr_j, thr_k) +
											 hit(matrixCopy, thr_i + 1, thr_j, thr_k) +
											 hit(matrixCopy, thr_i, thr_j - 1, thr_k) +
											 hit(matrixCopy, thr_i, thr_j + 1, thr_k) +
											 hit(matrixCopy, thr_i, thr_j, thr_k - 1) +
											 hit(matrixCopy, thr_i, thr_j, thr_k + 1))) /
									   42;

	CTRL_KERNEL_END();
}
