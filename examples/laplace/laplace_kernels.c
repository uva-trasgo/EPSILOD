/**
 * @file laplace_kernels.c
 * @brief Example for laplace equation.
 * 	Kernel code
 *
 * @copyright This software is part of the EPSILOD project by Trasgo Group, UVa.
 * The relevant license, warranty and copyright notice is available in the EPSILOD project repository.
 */

#include "laplace_ext_type.h"
#include "epsilod_kernels.h"

EPSILOD_KERNEL(initCell_3D, GENERIC, DEFAULT, KHitTile_float matrix, EpsilodCoords global_coords, Epsilod_ext ext_params, {
	// const HitInd i_g = thr_i + global_coords.offset[0];
	const HitInd j_g = thr_j + global_coords.offset[1];
	const HitInd k_g = thr_k + global_coords.offset[2];

	// Note: removed to imitate Devito example
	// There's a bug in Devito when applying boundary conditions in multiprocess
	// if (i_g < global_coords.borders.low[0])
	// 	hit(matrix, thr_i, thr_j, thr_k) = 1.;
	// else if (i_g >= global_coords.size[0] - global_coords.borders.high[0])
	// 	hit(matrix, thr_i, thr_j, thr_k) = 2.;
	// else
	if (j_g < global_coords.borders.low[1])
		hit(matrix, thr_i, thr_j, thr_k) = 3.;
	else if (j_g >= global_coords.size[1] - global_coords.borders.high[1])
		hit(matrix, thr_i, thr_j, thr_k) = 4.;
	else if (k_g < global_coords.borders.low[2])
		hit(matrix, thr_i, thr_j, thr_k) = 5.;
	else if (k_g >= global_coords.size[2] - global_coords.borders.high[2])
		hit(matrix, thr_i, thr_j, thr_k) = 6.;
	else
		hit(matrix, thr_i, thr_j, thr_k) = 0.;
});

/* KERNEL GENERIC: LAPLACE */
EPSILOD_KERNEL(updateCell_laplace, GENERIC, DEFAULT, KHitTile(EPSILOD_BASE_TYPE) matrix, const KHitTile(EPSILOD_BASE_TYPE) matrixCopy, EpsilodCoords global_coords, KHitTile_float stencil, float factor, const Epsilod_ext ext_params, {
	int               i  = thr_i;
	int               j  = thr_j;
	int               k  = thr_k;
	EPSILOD_BASE_TYPE r0 = ext_params.r0;
	EPSILOD_BASE_TYPE r1 = ext_params.r1;
	EPSILOD_BASE_TYPE r2 = ext_params.r2;
	EPSILOD_BASE_TYPE r3 = ext_params.r3;

	hit(matrix, i, j, k) =
		5.0e-1F * r0 *
		(r2 *
			 (r3 * hit(matrixCopy, i - 1, j, k) +
			  r3 * hit(matrixCopy, i + 1, j, k)) +
		 r1 *
			 (r2 * hit(matrixCopy, i, j, k - 1) +
			  r2 * hit(matrixCopy, i, j, k + 1) +
			  r3 * hit(matrixCopy, i, j - 1, k) +
			  r3 * hit(matrixCopy, i, j + 1, k)));
});
