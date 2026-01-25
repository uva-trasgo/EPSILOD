/**
 * @file basic_stencils/fpga_kernels/1dC2.cl
 * @brief Epsilod: Example with several key stencils. FPGA 1dC2 kernel code.
 *
 * @copyright This software is part of the EPSILOD project by Trasgo Group, UVa.
 * The relevant license, warranty and copyright notice is available in the EPSILOD project repository.
 */

#include "epsilod_types.h"

#include "Ctrl_FPGA_Kernel.h"

Ctrl_NewType(float);

// 1D compact radius 1
__CTRL_FPGA_KERNEL_OPTIMIZE_FOR(8, 256, 1, 1)
__CTRL_FPGA_KERNEL_REPLICATE(4)
CTRL_KERNEL_FN(updateCell_1dC2, FPGA, NDRANGE, KHitTileR_arg(float, matrix), KHitTileR_arg(float, matrixCopy),
			   K_arg(EpsilodCoords, global_coords), KHitTileR_arg(float, stencil), K_arg(float, factor),
			   K_arg(Epsilod_ext, ext_params)) {
	hit(matrix, thr_i) = (hit(matrixCopy, thr_i - 1) + hit(matrixCopy, thr_i + 1)) / 2;

	CTRL_KERNEL_END();
}

__CTRL_FPGA_KERNEL_REPLICATE(4)
CTRL_KERNEL_FN(updateCell_1dC2_border, FPGA, NDRANGE, KHitTileR_arg(float, matrix), KHitTileR_arg(float, matrixCopy),
			   K_arg(EpsilodCoords, global_coords), KHitTileR_arg(float, stencil), K_arg(float, factor),
			   K_arg(Epsilod_ext, ext_params)) {
	hit(matrix, thr_i) = (hit(matrixCopy, thr_i - 1) + hit(matrixCopy, thr_i + 1)) / 2;

	CTRL_KERNEL_END();
}
