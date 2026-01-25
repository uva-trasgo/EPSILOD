/**
 * @file fpga_kernels/epsilod/1d.cl
 * @brief Epsilod: Generic FPGA kernel for fully described 1D stencils.
 *
 * @copyright This software is part of the EPSILOD project by Trasgo Group, UVa.
 * The relevant license, warranty and copyright notice is available in the EPSILOD project repository.
 */

#include "epsilod_fpga_kernels.h"

#if EPSILOD_IS_FLOAT(EPSILOD_BASE_TYPE)
/* 1D cell update default stencil */
__CTRL_FPGA_KERNEL_OPTIMIZE_FOR(8, 256, 1, 1)
__CTRL_FPGA_KERNEL_REPLICATE(4)
CTRL_KERNEL_FN(updateCell_default_1D, FPGA, NDRANGE, KHitTileR_arg(float, matrix), KHitTileR_arg(float, matrixCopy),
			   K_arg(EpsilodCoords, global_coords), KHitTileR_arg(float, weight), K_arg(float, factor),
			   K_arg(Epsilod_ext, ext_params)) {
	int begin_i = -global_coords.borders.low[0];
	int end_i   = global_coords.borders.high[0];

	float sum = 0;

	for (int i = begin_i; i <= end_i; i++) {
		// if (!hit(weight, i - begin_i)) continue;
		sum += hit(matrixCopy, thr_i + i) * hit(weight, i - begin_i);
	}
	hit(matrix, thr_i) = sum / factor;

	CTRL_KERNEL_END();
}
#endif // EPSILOD_IS_FLOAT(EPSILOD_BASE_TYPE)
