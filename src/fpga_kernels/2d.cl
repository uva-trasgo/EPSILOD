/**
 * @file fpga_kernels/epsilod/2d.cl
 * @brief Epsilod: Generic FPGA kernel for fully described 2D stencils.
 *
 * @copyright This software is part of the EPSILOD project by Trasgo Group, UVa.
 * The relevant license, warranty and copyright notice is available in the EPSILOD project repository.
 */

#include "epsilod_fpga_kernels.h"

#if EPSILOD_IS_FLOAT(EPSILOD_BASE_TYPE)
/* 2D cell update default stencil */
__CTRL_FPGA_KERNEL_OPTIMIZE_FOR(8, 64, 4, 1)
__CTRL_FPGA_KERNEL_REPLICATE(2, 2)
CTRL_KERNEL_FN(updateCell_default_2D, FPGA, NDRANGE, KHitTileR_arg(float, matrix), KHitTileR_arg(float, matrixCopy),
			   K_arg(EpsilodCoords, global_coords), KHitTileR_arg(float, weight), K_arg(float, factor),
			   K_arg(Epsilod_ext, ext_params)) {
	int begin_i = -global_coords.borders.low[0];
	int end_i   = global_coords.borders.high[0];
	int begin_j = -global_coords.borders.low[1];
	int end_j   = global_coords.borders.high[1];

	float sum = 0;

	for (int i = begin_i; i <= end_i; i++)
		for (int j = begin_j; j <= end_j; j++) {
			// if (!hit(weight, i - begin_i, j - begin_j)) continue;
			sum += hit(matrixCopy, thr_i + i, thr_j + j) * hit(weight, i - begin_i, j - begin_j);
		}
	hit(matrix, thr_i, thr_j) = sum / factor;

	CTRL_KERNEL_END();
}
#endif // EPSILOD_IS_FLOAT(EPSILOD_BASE_TYPE)
