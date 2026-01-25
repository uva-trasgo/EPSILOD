/**
 * @file gaussian_kernels.c
 * @brief Example for gaussian blur filter. Kernel code.
 *
 * @copyright This software is part of the EPSILOD project by Trasgo Group, UVa.
 * The relevant license, warranty and copyright notice is available in the EPSILOD project repository.
 *
 * @copyright This file is part of a modified version of a Muesli example. Thus the following applies:
 * @copyright Copyright (c) 2016
 * 		Steffen Ernsting <s.ernsting@uni-muenster.de>,
 * 		Herbert Kuchen <kuchen@uni-muenster.de>,
 * 		Fabian Wrede <fabian.wrede@wi.uni-muenster.de>.
 *
 * @copyright Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * @copyright The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 */

#include <math.h>

#include "gaussian_ext_type.h"
#include "epsilod_kernels.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846 /* pi */
#endif // M_PI

/* KERNEL GENERIC: GAUSSIAN BLUR */
CTRL_KERNEL(updateCell_gaussian, GENERIC, DEFAULT, KHitTile(EPSILOD_BASE_TYPE) matrix, const KHitTile(EPSILOD_BASE_TYPE) matrixCopy, EpsilodCoords global_coords, KHitTile_float stencil, float factor, const Epsilod_ext ext_params, {
	int x = thr_i;
	int y = thr_j;

	// #define DEBUG_GAUSSIAN
	// DEBUG: COPY KERNEL, NO TRANSFORMATION
	#ifdef DEBUG_GAUSSIAN
	EPSILOD_BASE_TYPE sum = hit(matrixCopy, x, y);
	#else
	int kw = ext_params.kw;

	int               offset = kw / 2;
	EPSILOD_BASE_TYPE mean   = (EPSILOD_BASE_TYPE)kw / 2;
	EPSILOD_BASE_TYPE sigma  = 1;
	EPSILOD_BASE_TYPE sum    = 0.0f;
	for (int r = 0; r < kw; ++r) {
		for (int c = 0; c < kw; ++c) {
			sum += hit(matrixCopy, x + r - offset, y + c - offset) *
				   expf(-0.5f * (powf((r - mean) / sigma, 2.0) + powf((c - mean) / sigma, 2.0))) / (2 * M_PI * sigma * sigma);
		}
	}
	#endif
	hit(matrix, x, y) = (int)sum; // divided by weight, but weight is constant 1.0f
});
