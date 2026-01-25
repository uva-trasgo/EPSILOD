/**
 * @file fpga_kernels/epsilod/3d.cl
 * @brief Epsilod: Generic FPGA kernel for fully described 3D stencils.
 *
 * @copyright This software is part of the EPSILOD project by Trasgo Group, UVa.
 * The relevant license, warranty and copyright notice is available in the EPSILOD project repository.
 */

#include "epsilod_fpga_kernels.h"

#if EPSILOD_IS_FLOAT(EPSILOD_BASE_TYPE)
/* 3D cell update default stencil */
__CTRL_FPGA_KERNEL_OPTIMIZE_FOR(8, 64, 4, 1)
__CTRL_FPGA_KERNEL_REPLICATE(2, 2, 2)
CTRL_KERNEL_FN(updateCell_default_3D, FPGA, NDRANGE, KHitTileR_arg(float, matrix), KHitTileR_arg(float, matrixCopy),
			   K_arg(EpsilodCoords, global_coords), KHitTileR_arg(float, weight), K_arg(float, factor),
			   K_arg(Epsilod_ext, ext_params)) {
	int begin_i = -global_coords.borders.low[0];
	int end_i   = global_coords.borders.high[0];
	int begin_j = -global_coords.borders.low[1];
	int end_j   = global_coords.borders.high[1];
	int begin_k = -global_coords.borders.low[2];
	int end_k   = global_coords.borders.high[2];

	float sum = 0;

	for (int i = begin_i; i <= end_i; i++)
		for (int j = begin_j; j <= end_j; j++)
			for (int k = begin_k; k <= end_k; k++) {
				// if (!hit(weight, i - begin_i, j - begin_j, k - begin_k)) continue;
				sum += hit(matrixCopy, thr_i + i, thr_j + j, thr_k + k) * hit(weight, i - begin_i, j - begin_j, k - begin_k);
			}
	hit(matrix, thr_i, thr_j, thr_k) = sum / factor;

	CTRL_KERNEL_END();
}
#endif // EPSILOD_IS_FLOAT(EPSILOD_BASE_TYPE)
