/**
 * @file laplace_kernels.c
 * @brief Example for laplace equation.
 * 	Kernel code
 *
 * @copyright This software is part of the EPSILOD project by Trasgo Group, UVa.
 * The relevant license, warranty and copyright notice is available in the EPSILOD project repository.
 */

#include <math.h>

#include "laplace_ext_type.h"
#include "epsilod_kernels.h"

#if EPSILOD_IS_FLOAT(EPSILOD_BASE_TYPE)
#define POW(a, b) powf(a, b)
#else // !EPSILOD_IS_FLOAT(EPSILOD_BASE_TYPE)
#define POW(a, b) pow(a, b)
#endif // EPSILOD_IS_FLOAT(EPSILOD_BASE_TYPE)

/* KERNEL GENERIC: LAPLACE */
CTRL_KERNEL(noop_laplace, GENERIC, DEFAULT, KHitTile(EPSILOD_BASE_TYPE) matrix, const KHitTile(EPSILOD_BASE_TYPE) matrixCopy, EpsilodCoords global_coords, KHitTile_float stencil, float factor, const Epsilod_ext ext_params, {
	hit(matrix, thr_i, thr_j) = hit(matrixCopy, thr_i, thr_j);
});

/* KERNEL GENERIC: LAPLACE */
CTRL_KERNEL(updateCell_laplace, GENERIC, DEFAULT, KHitTile(EPSILOD_BASE_TYPE) matrix, const KHitTile(EPSILOD_BASE_TYPE) matrixCopy, EpsilodCoords global_coords, KHitTile_float stencil, float factor, const Epsilod_ext ext_params, {
	int i = thr_i;
	int j = thr_j;

	EPSILOD_BASE_TYPE dx = ext_params.dx;
	EPSILOD_BASE_TYPE dy = ext_params.dy;

	// int x_g = thr_i + global.offset[0];
	// int y_g = thr_j + global.offset[1];

	// if (x_g == 1 && y_g == 2)
	// 	hit(matrix, i, j) = hit(matrixCopy, i, j) + 1;

	hit(matrix, i, j) = (POW(dy, 2) *
							 (hit(matrixCopy, i - 1, j) +
							  hit(matrixCopy, i + 1, j)) +
						 POW(dx, 2) *
							 (hit(matrixCopy, i, j - 1) +
							  hit(matrixCopy, i, j + 1))) /
						(2 * (POW(dx, 2) + POW(dy, 2)));
});
