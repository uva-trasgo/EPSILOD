/**
 * @file basic_stencils_kernels.c
 * @brief Epsilod: Example with several key stencils. Kernels code.
 *
 * @copyright This software is part of the EPSILOD project by Trasgo Group, UVa.
 * The relevant license, warranty and copyright notice is available in the EPSILOD project repository.
 */

#include "epsilod_types.h"
#include "epsilod_kernels.h"

EPSILOD_KERNEL(initCell_1D, GENERIC, DEFAULT, KHitTile_float matrix, EpsilodCoords global_coords, Epsilod_ext ext_params, {
	const HitInd i_g = thr_i + global_coords.offset[0];

	if (i_g < global_coords.borders.low[0])
		hit(matrix, thr_i, thr_j) = 1.;
	else if (i_g >= global_coords.size[0] - global_coords.borders.high[0])
		hit(matrix, thr_i, thr_j) = 2.;
	else
		hit(matrix, thr_i, thr_j) = 0.;
});

EPSILOD_KERNEL(initCell_2D, GENERIC, DEFAULT, KHitTile_float matrix, EpsilodCoords global_coords, Epsilod_ext ext_params, {
	const HitInd i_g = thr_i + global_coords.offset[0];
	const HitInd j_g = thr_j + global_coords.offset[1];

	if (i_g < global_coords.borders.low[0])
		hit(matrix, thr_i, thr_j) = 1.;
	else if (i_g >= global_coords.size[0] - global_coords.borders.high[0])
		hit(matrix, thr_i, thr_j) = 2.;
	else if (j_g < global_coords.borders.low[1])
		hit(matrix, thr_i, thr_j) = 3.;
	else if (j_g >= global_coords.size[1] - global_coords.borders.high[1])
		hit(matrix, thr_i, thr_j) = 4.;
	else
		hit(matrix, thr_i, thr_j) = 0.;
});

EPSILOD_KERNEL(initCell_3D, GENERIC, DEFAULT, KHitTile_float matrix, EpsilodCoords global_coords, Epsilod_ext ext_params, {
	const HitInd i_g = thr_i + global_coords.offset[0];
	const HitInd j_g = thr_j + global_coords.offset[1];
	const HitInd k_g = thr_k + global_coords.offset[2];

	if (i_g < global_coords.borders.low[0])
		hit(matrix, thr_i, thr_j, thr_k) = 1.;
	else if (i_g >= global_coords.size[0] - global_coords.borders.high[0])
		hit(matrix, thr_i, thr_j, thr_k) = 2.;
	else if (j_g < global_coords.borders.low[1])
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

EPSILOD_KERNEL(initCell_4D, GENERIC, DEFAULT, KHitTile_float matrix, EpsilodCoords global_coords, Epsilod_ext ext_params, {
	const HitInd i_g = thr_i + global_coords.offset[1];
	const HitInd j_g = thr_j + global_coords.offset[2];
	const HitInd k_g = thr_k + global_coords.offset[3];

	for (int h = 0; h < hit_tileDimCard(matrix, 0); h++) {
		const HitInd h_g = h + global_coords.offset[0];
		if (h_g < global_coords.borders.low[0])
			hit(matrix, h, thr_i, thr_j, thr_k) = 1.;
		else if (h_g >= global_coords.size[0] - global_coords.borders.high[0])
			hit(matrix, h, thr_i, thr_j, thr_k) = 2.;
		else if (i_g < global_coords.borders.low[1])
			hit(matrix, h, thr_i, thr_j, thr_k) = 3.;
		else if (i_g >= global_coords.size[1] - global_coords.borders.high[1])
			hit(matrix, h, thr_i, thr_j, thr_k) = 4.;
		else if (j_g < global_coords.borders.low[2])
			hit(matrix, h, thr_i, thr_j, thr_k) = 5.;
		else if (j_g >= global_coords.size[2] - global_coords.borders.high[2])
			hit(matrix, h, thr_i, thr_j, thr_k) = 6.;
		else if (k_g < global_coords.borders.low[3])
			hit(matrix, h, thr_i, thr_j, thr_k) = 7.;
		else if (k_g >= global_coords.size[3] - global_coords.borders.high[3])
			hit(matrix, h, thr_i, thr_j, thr_k) = 8.;
		else
			hit(matrix, h, thr_i, thr_j, thr_k) = 0.;
	}
});

// 1D compact radius 1
EPSILOD_KERNEL(updateCell_1dC2, GENERIC, DEFAULT, KHitTileR_float matrix, const KHitTileR_float matrixCopy, const EpsilodCoords global_coords, const KHitTileR_float stencil, const float factor, const Epsilod_ext ext_params, {
	hit(matrix, thr_i) =
		(hit(matrixCopy, thr_i - 1) +
		 hit(matrixCopy, thr_i + 1)) /
		2;
});

// 1D non-compact radius 2
EPSILOD_KERNEL(updateCell_1dNC4, GENERIC, DEFAULT, KHitTileR_float matrix, const KHitTileR_float matrixCopy, const EpsilodCoords global_coords, const KHitTileR_float stencil, const float factor, const Epsilod_ext ext_params, {
	hit(matrix, thr_i) =
		(0.5 * (hit(matrixCopy, thr_i - 2) +
				hit(matrixCopy, thr_i + 2)) +
		 hit(matrixCopy, thr_i - 1) +
		 hit(matrixCopy, thr_i + 1)) /
		3;
});

// 2D compact, radius 1: 4-point star, no corners
EPSILOD_KERNEL(updateCell_2d4, GENERIC, DEFAULT, KHitTileR_float matrix, const KHitTileR_float matrixCopy, const EpsilodCoords global_coords, const KHitTileR_float stencil, const float factor, const Epsilod_ext ext_params, {
	hit(matrix, thr_i, thr_j) =
		(hit(matrixCopy, thr_i - 1, thr_j) +
		 hit(matrixCopy, thr_i + 1, thr_j) +
		 hit(matrixCopy, thr_i, thr_j - 1) +
		 hit(matrixCopy, thr_i, thr_j + 1)) /
		4;
});

// 2D compact, radius 1: 8-point star, corners included
EPSILOD_KERNEL(updateCell_2d8, GENERIC, DEFAULT, KHitTileR_float matrix, const KHitTileR_float matrixCopy, const EpsilodCoords global_coords, const KHitTileR_float stencil, const float factor, const Epsilod_ext ext_params, {
	hit(matrix, thr_i, thr_j) =
		(4 * (hit(matrixCopy, thr_i - 1, thr_j) +
			  hit(matrixCopy, thr_i + 1, thr_j) +
			  hit(matrixCopy, thr_i, thr_j - 1) +
			  hit(matrixCopy, thr_i, thr_j + 1)) +
		 (hit(matrixCopy, thr_i - 1, thr_j - 1) +
		  hit(matrixCopy, thr_i + 1, thr_j - 1) +
		  hit(matrixCopy, thr_i - 1, thr_j + 1) +
		  hit(matrixCopy, thr_i + 1, thr_j + 1))) /
		20;
});

// 2D non-compact, radius 2: 8-point star, no corners
EPSILOD_KERNEL(updateCell_2dNC8, GENERIC, DEFAULT, KHitTileR_float matrix, const KHitTileR_float matrixCopy, const EpsilodCoords global_coords, const KHitTileR_float stencil, const float factor, const Epsilod_ext ext_params, {
	hit(matrix, thr_i, thr_j) =
		((hit(matrixCopy, thr_i - 2, thr_j) +
		  hit(matrixCopy, thr_i + 2, thr_j) +
		  hit(matrixCopy, thr_i, thr_j - 2) +
		  hit(matrixCopy, thr_i, thr_j + 2)) +
		 4 * (hit(matrixCopy, thr_i - 1, thr_j) +
			  hit(matrixCopy, thr_i + 1, thr_j) +
			  hit(matrixCopy, thr_i, thr_j - 1) +
			  hit(matrixCopy, thr_i, thr_j + 1))) /
		20;
});

// 2D non-compact, non-symmetric. radius 2: 5-point star forward-down with one corner element
EPSILOD_KERNEL(updateCell_2dF5, GENERIC, DEFAULT, KHitTileR_float matrix, const KHitTileR_float matrixCopy, const EpsilodCoords global_coords, const KHitTileR_float stencil, const float factor, const Epsilod_ext ext_params, {
	hit(matrix, thr_i, thr_j) =
		(2.0f * (hit(matrixCopy, thr_i - 1, thr_j) +
				 hit(matrixCopy, thr_i, thr_j - 1)) +
		 (hit(matrixCopy, thr_i - 2, thr_j) +
		  hit(matrixCopy, thr_i, thr_j - 2)) +
		 .5f * hit(matrixCopy, thr_i - 1, thr_j - 1)) /
		6.5f;
});

// 3D compact. radius 1: 27-point star
EPSILOD_KERNEL(updateCell_3d27, GENERIC, DEFAULT, KHitTileR_float matrix, const KHitTileR_float matrixCopy, const EpsilodCoords global_coords, const KHitTileR_float stencil, const float factor, const Epsilod_ext ext_params, {
	hit(matrix, thr_i, thr_j, thr_k) =
		(hit(matrixCopy, thr_i - 1, thr_j - 1, thr_k - 1) +
		 hit(matrixCopy, thr_i - 1, thr_j - 1, thr_k) +
		 hit(matrixCopy, thr_i - 1, thr_j - 1, thr_k + 1) +
		 hit(matrixCopy, thr_i - 1, thr_j, thr_k - 1) +
		 hit(matrixCopy, thr_i - 1, thr_j, thr_k) +
		 hit(matrixCopy, thr_i - 1, thr_j, thr_k + 1) +
		 hit(matrixCopy, thr_i - 1, thr_j + 1, thr_k - 1) +
		 hit(matrixCopy, thr_i - 1, thr_j + 1, thr_k) +
		 hit(matrixCopy, thr_i - 1, thr_j + 1, thr_k + 1) +
		 hit(matrixCopy, thr_i, thr_j - 1, thr_k - 1) +
		 hit(matrixCopy, thr_i, thr_j - 1, thr_k) +
		 hit(matrixCopy, thr_i, thr_j - 1, thr_k + 1) +
		 hit(matrixCopy, thr_i, thr_j, thr_k - 1) +
		 hit(matrixCopy, thr_i, thr_j, thr_k) +
		 hit(matrixCopy, thr_i, thr_j, thr_k + 1) +
		 hit(matrixCopy, thr_i, thr_j + 1, thr_k - 1) +
		 hit(matrixCopy, thr_i, thr_j + 1, thr_k) +
		 hit(matrixCopy, thr_i, thr_j + 1, thr_k + 1) +
		 hit(matrixCopy, thr_i + 1, thr_j - 1, thr_k - 1) +
		 hit(matrixCopy, thr_i + 1, thr_j - 1, thr_k) +
		 hit(matrixCopy, thr_i + 1, thr_j - 1, thr_k + 1) +
		 hit(matrixCopy, thr_i + 1, thr_j, thr_k - 1) +
		 hit(matrixCopy, thr_i + 1, thr_j, thr_k) +
		 hit(matrixCopy, thr_i + 1, thr_j, thr_k + 1) +
		 hit(matrixCopy, thr_i + 1, thr_j + 1, thr_k - 1) +
		 hit(matrixCopy, thr_i + 1, thr_j + 1, thr_k) +
		 hit(matrixCopy, thr_i + 1, thr_j + 1, thr_k + 1)) /
		27;
});

// 3D compact. radius 1: 6-point star
EPSILOD_KERNEL(updateCell_3d6, GENERIC, DEFAULT, KHitTileR_float matrix, const KHitTileR_float matrixCopy, const EpsilodCoords global_coords, const KHitTileR_float stencil, const float factor, const Epsilod_ext ext_params, {
	hit(matrix, thr_i, thr_j, thr_k) =
		(hit(matrixCopy, thr_i - 1, thr_j, thr_k) +
		 hit(matrixCopy, thr_i + 1, thr_j, thr_k) +
		 hit(matrixCopy, thr_i, thr_j - 1, thr_k) +
		 hit(matrixCopy, thr_i, thr_j + 1, thr_k) +
		 hit(matrixCopy, thr_i, thr_j, thr_k - 1) +
		 hit(matrixCopy, thr_i, thr_j, thr_k + 1)) /
		6;
});

// 3D non-compact, radius 2: 12-point star, no corners
EPSILOD_KERNEL(updateCell_3dNC12, GENERIC, DEFAULT, KHitTileR_float matrix, const KHitTileR_float matrixCopy, const EpsilodCoords global_coords, const KHitTileR_float stencil, const float factor, const Epsilod_ext ext_params, {
	hit(matrix, thr_i, thr_j, thr_k) =
		(hit(matrixCopy, thr_i - 2, thr_j, thr_k) +
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
});

// 3D non-compact, non-symmetric, radius 2: 7-point star forward-down-below with one corner element
EPSILOD_KERNEL(updateCell_3dF7, GENERIC, DEFAULT, KHitTileR_float matrix, const KHitTileR_float matrixCopy, const EpsilodCoords global_coords, const KHitTileR_float stencil, const float factor, const Epsilod_ext ext_params, {
	hit(matrix, thr_i, thr_j, thr_k) =
		(2.0f * (hit(matrixCopy, thr_i - 1, thr_j, thr_k) +
				 hit(matrixCopy, thr_i, thr_j - 1, thr_k) +
				 hit(matrixCopy, thr_i, thr_j, thr_k - 1)) +
		 hit(matrixCopy, thr_i - 2, thr_j, thr_k) +
		 hit(matrixCopy, thr_i, thr_j - 2, thr_k) +
		 hit(matrixCopy, thr_i, thr_j, thr_k - 2) +
		 .5f * hit(matrixCopy, thr_i - 1, thr_j - 1, thr_k - 1)) /
		9.5f;
});

// 4D compact. radius 1: 8-point star
EPSILOD_KERNEL(updateCell_4d8, GENERIC, DEFAULT, KHitTileR_float matrix, const KHitTileR_float matrixCopy, const EpsilodCoords global_coords, const KHitTileR_float stencil, const float factor, const Epsilod_ext ext_params, {
	for (int h = 0; h < hit_tileDimCard(matrix, 0); h++)
		hit(matrix, h, thr_i, thr_j, thr_k) =
			(hit(matrixCopy, h - 1, thr_i, thr_j, thr_k) +
			 hit(matrixCopy, h + 1, thr_i, thr_j, thr_k) +
			 hit(matrixCopy, h, thr_i - 1, thr_j, thr_k) +
			 hit(matrixCopy, h, thr_i + 1, thr_j, thr_k) +
			 hit(matrixCopy, h, thr_i, thr_j - 1, thr_k) +
			 hit(matrixCopy, h, thr_i, thr_j + 1, thr_k) +
			 hit(matrixCopy, h, thr_i, thr_j, thr_k - 1) +
			 hit(matrixCopy, h, thr_i, thr_j, thr_k + 1)) /
			8;
});
