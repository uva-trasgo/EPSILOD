/**
 * @file fpga_kernels/epsilod/4d.cl
 * @brief Epsilod: Generic FPGA kernel for fully described 4D stencils.
 *
 * @copyright This software is part of the EPSILOD project by Trasgo Group, UVa.
 * The relevant license, warranty and copyright notice is available in the EPSILOD project repository.
 */

#include "epsilod_fpga_kernels.h"

#if EPSILOD_IS_FLOAT(EPSILOD_BASE_TYPE)
/* 4D cell update default stencil */
__CTRL_FPGA_KERNEL_OPTIMIZE_FOR(8, 64, 4, 1)
__CTRL_FPGA_KERNEL_REPLICATE(2, 2, 2)
CTRL_KERNEL_FN(updateCell_default_4D, FPGA, NDRANGE, KHitTileR_arg(float, matrix), KHitTileR_arg(float, matrixCopy),
			   K_arg(EpsilodCoords, global_coords), KHitTileR_arg(float, weight), K_arg(float, factor),
			   K_arg(Epsilod_ext, ext_params)) {
	int begin_i = -global_coords.borders.low[0];
	int end_i   = global_coords.borders.high[0];
	int begin_j = -global_coords.borders.low[1];
	int end_j   = global_coords.borders.high[1];
	int begin_k = -global_coords.borders.low[2];
	int end_k   = global_coords.borders.high[2];
	int begin_l = -global_coords.borders.low[3];
	int end_l   = global_coords.borders.high[3];

	for (int gl = 0; gl < hit_tileDimCard(matrix, 3); gl++) {
		float sum = 0;
		for (int i = begin_i; i <= end_i; i++)
			for (int j = begin_j; j <= end_j; j++)
				for (int k = begin_k; k <= end_k; k++)
					for (int l = begin_l; l <= end_l; l++) {
						// if (!hit(weight, i - begin_i, j - begin_j, k - begin_k, l - begin_l)) continue;
						sum += hit(matrixCopy, thr_i + i, thr_j + j, thr_k + k, gl + l) * hit(weight, i - begin_i, j - begin_j, k - begin_k, l - begin_l);
					}
		hit(matrix, thr_i, thr_j, thr_k, gl) = sum / factor;
	}

	CTRL_KERNEL_END();
}
#endif // EPSILOD_IS_FLOAT(EPSILOD_BASE_TYPE)
