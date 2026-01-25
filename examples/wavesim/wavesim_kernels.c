/**
 * @file wavesim_kernels.c
 * @brief Example for wave simulation. Kernel code.
 *
 * @copyright This software is part of the EPSILOD project by Trasgo Group, UVa.
 * The relevant license, warranty and copyright notice is available in the EPSILOD project repository.
 */

#include <math.h>

#include "wavesim_types.h"
#include "wavesim_ext_type.h"

#include "epsilod_kernels.h"

/* KERNEL GENERIC: WAVESIM INIT */
CTRL_KERNEL(initCell_wavesim, GENERIC, DEFAULT, KHitTile_float matrix, EpsilodCoords global_coords, Epsilod_ext ext_params, {
	int radius = 1;

	const int y_g = thr_i + global_coords.offset[0] - radius;
	const int x_g = thr_j + global_coords.offset[1] - radius;

	const vec2f *const      s  = &ext_params.sigma;
	const EPSILOD_BASE_TYPE a  = ext_params.amplitude;
	const float             dx = x_g - ext_params.center.x;
	const float             dy = y_g - ext_params.center.y;

	hit(matrix, thr_i, thr_j) = a * expf(-(dx * dx / (2.f * s->x * s->x) + dy * dy / (2.f * s->y * s->y)));
});

/* KERNEL GENERIC: WAVESIM INIT COPY */
CTRL_KERNEL(initCellCopy_wavesim, GENERIC, DEFAULT, KHitTile(EPSILOD_BASE_TYPE) matrix, const KHitTile(EPSILOD_BASE_TYPE) matrixCopy, EpsilodCoords global_coords, KHitTile_float stencil, float factor, const Epsilod_ext ext_params, {
	const int y = thr_i;
	const int x = thr_j;

	const EPSILOD_BASE_TYPE dt = ext_params.dt;
	const EPSILOD_BASE_TYPE dx = ext_params.dx;
	const EPSILOD_BASE_TYPE dy = ext_params.dy;

	int radius = 1;

	const int y_g = y + global_coords.offset[0] - radius;
	const int x_g = x + global_coords.offset[1] - radius;

	const vec2l size = ((vec2l){global_coords.size[1] - 2 * radius, global_coords.size[0] - 2 * radius});

	const int py = y_g < size.y - 1 ? y + 1 : y;
	const int my = y_g > 0 ? y - 1 : y;
	const int px = x_g < size.x - 1 ? x + 1 : x;
	const int mx = x_g > 0 ? x - 1 : x;

	const float lap =
		(dt / dy) * (dt / dy) *
			((hit(matrixCopy, py, x) - hit(matrixCopy, y, x)) -
			 (hit(matrixCopy, y, x) - hit(matrixCopy, my, x))) +
		(dt / dx) * (dt / dx) *
			((hit(matrixCopy, y, px) - hit(matrixCopy, y, x)) -
			 (hit(matrixCopy, y, x) - hit(matrixCopy, y, mx)));

	hit(matrix, y, x) = hit(matrixCopy, y, x) + 0.5 * lap;
});

/* KERNEL GENERIC: WAVESIM */
CTRL_KERNEL(updateCell_wavesim, GENERIC, DEFAULT, KHitTile(EPSILOD_BASE_TYPE) matrix, const KHitTile(EPSILOD_BASE_TYPE) matrixCopy, EpsilodCoords global_coords, KHitTile_float stencil, float factor, const Epsilod_ext ext_params, {
	const int y = thr_i;
	const int x = thr_j;

	const EPSILOD_BASE_TYPE dt = ext_params.dt;
	const EPSILOD_BASE_TYPE dx = ext_params.dx;
	const EPSILOD_BASE_TYPE dy = ext_params.dy;

	int radius = 1;

	const int y_g = y + global_coords.offset[0] - radius;
	const int x_g = x + global_coords.offset[1] - radius;

	const vec2l size = ((vec2l){global_coords.size[1] - 2 * radius, global_coords.size[0] - 2 * radius});

	const int py = y_g < size.y - 1 ? y + 1 : y;
	const int my = y_g > 0 ? y - 1 : y;
	const int px = x_g < size.x - 1 ? x + 1 : x;
	const int mx = x_g > 0 ? x - 1 : x;

	const float lap =
		(dt / dy) * (dt / dy) *
			((hit(matrixCopy, py, x) - hit(matrixCopy, y, x)) -
			 (hit(matrixCopy, y, x) - hit(matrixCopy, my, x))) +
		(dt / dx) * (dt / dx) *
			((hit(matrixCopy, y, px) - hit(matrixCopy, y, x)) -
			 (hit(matrixCopy, y, x) - hit(matrixCopy, y, mx)));

	hit(matrix, y, x) = 2 * hit(matrixCopy, y, x) - hit(matrix, y, x) + lap;
});
