/**
 * @file epsilod_kernels.cu
 * @brief Epsilod: Generic kernels for fully described 1D, 2D, 3D stencils.
 *
 * @copyright This software is part of the EPSILOD project by Trasgo Group, UVa.
 * The relevant license, warranty and copyright notice is available in the EPSILOD project repository.
 */

#include "epsilod_kernels.h"

#if EPSILOD_IS_FLOAT(EPSILOD_BASE_TYPE)
/* 1D cell update default stencil */
CTRL_KERNEL(updateCell_default_1D, GENERIC, DEFAULT, KHitTileR_float matrix, const KHitTileR_float matrixCopy, EpsilodCoords global_coords, const KHitTileR_float weight, const float factor, const Epsilod_ext ext_params, {
	int begin_i = -global_coords.borders.low[0];
	int end_i   = global_coords.borders.high[0];

	float sum = 0;

	for (int i = begin_i; i <= end_i; i++) {
		if (!hit(weight, i - begin_i)) continue;
		sum += hit(matrixCopy, thr_i + i) * hit(weight, i - begin_i);
	}
	hit(matrix, thr_i) = sum / factor;
});

/* 2D cell update default stencil */
CTRL_KERNEL(updateCell_default_2D, GENERIC, DEFAULT, KHitTileR_float matrix, const KHitTileR_float matrixCopy, EpsilodCoords global_coords, const KHitTileR_float weight, const float factor, const Epsilod_ext ext_params, {
	int begin_i = -global_coords.borders.low[0];
	int end_i   = global_coords.borders.high[0];
	int begin_j = -global_coords.borders.low[1];
	int end_j   = global_coords.borders.high[1];

	float sum = 0;

	for (int i = begin_i; i <= end_i; i++)
		for (int j = begin_j; j <= end_j; j++) {
			if (!hit(weight, i - begin_i, j - begin_j)) continue;
			sum += hit(matrixCopy, thr_i + i, thr_j + j) * hit(weight, i - begin_i, j - begin_j);
		}
	hit(matrix, thr_i, thr_j) = sum / factor;
});

/* 3D cell update default stencil */
CTRL_KERNEL(updateCell_default_3D, GENERIC, DEFAULT, KHitTileR_float matrix, const KHitTileR_float matrixCopy, EpsilodCoords global_coords, const KHitTileR_float weight, const float factor, const Epsilod_ext ext_params, {
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
				if (!hit(weight, i - begin_i, j - begin_j, k - begin_k)) continue;
				sum += hit(matrixCopy, thr_i + i, thr_j + j, thr_k + k) * hit(weight, i - begin_i, j - begin_j, k - begin_k);
			}
	hit(matrix, thr_i, thr_j, thr_k) = sum / factor;
});

/* 4D cell update default stencil */
CTRL_KERNEL(updateCell_default_4D, GENERIC, DEFAULT, KHitTileR_float matrix, const KHitTileR_float matrixCopy, EpsilodCoords global_coords, const KHitTileR_float weight, const float factor, const Epsilod_ext ext_params, {
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
						if (!hit(weight, i - begin_i, j - begin_j, k - begin_k, l - begin_l)) continue;
						sum += hit(matrixCopy, thr_i + i, thr_j + j, thr_k + k, gl + l) * hit(weight, i - begin_i, j - begin_j, k - begin_k, l - begin_l);
					}
		hit(matrix, thr_i, thr_j, thr_k, gl) = sum / factor;
	}
});
#endif // EPSILOD_IS_FLOAT(EPSILOD_BASE_TYPE)

/* Copy kernel for device initialization */
CTRL_KERNEL(epsilod_dev_copy_1d, GENERIC, DEFAULT, KHitTileR(EPSILOD_BASE_TYPE) matrix, const KHitTileR(EPSILOD_BASE_TYPE) matrix_out, {
	hit(matrix_out, thr_i) = hit(matrix, thr_i);
});

CTRL_KERNEL(epsilod_dev_copy_2d, GENERIC, DEFAULT, KHitTileR(EPSILOD_BASE_TYPE) matrix, const KHitTileR(EPSILOD_BASE_TYPE) matrix_out, {
	hit(matrix_out, thr_i, thr_j) = hit(matrix, thr_i, thr_j);
});

CTRL_KERNEL(epsilod_dev_copy_3d, GENERIC, DEFAULT, KHitTileR(EPSILOD_BASE_TYPE) matrix, const KHitTileR(EPSILOD_BASE_TYPE) matrix_out, {
	hit(matrix_out, thr_i, thr_j, thr_k) = hit(matrix, thr_i, thr_j, thr_k);
});

/* Empty kernel: to signal subselection and root tiles as modified to track dependencies */
CTRL_KERNEL(epsilod_dev_touch, GENERIC, DEFAULT, KHitTileR(EPSILOD_BASE_TYPE) matrix, { ; });
