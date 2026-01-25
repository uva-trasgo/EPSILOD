/**
 * @file basic_stencils/fpga_kernels/kernel_body/2dF5opt.h
 * @brief File to be included multiple times inside 2dF5opt.cl, to avoid repetition of hard-to-debug code.
 *
 * This file expects x_size and y_size to be defined int variables in the file where it is included.
 * It also expects all the UPPER_SNAKE_CASE symbols to be defined constant preprocessor values.
 *
 * @copyright This software is part of the EPSILOD project by Trasgo Group, UVa.
 * The relevant license, warranty and copyright notice is available in the EPSILOD project repository.
 */

/* Default definitions for macros exclusive to this file: */

// Macro to signal that more than one block in each dimension will be computed,
// manages certain logic
#ifndef MORE_THAN_ONE_BLOCK
#define MORE_THAN_ONE_BLOCK true
#endif // MORE_THAN_ONE_BLOCK

// Shift register:
// Stores a partial matrix to reuse data for neighbor accesses
float shift_register[SR_SIZE];

// Pre-initialize the shift-register with zeroes:
#pragma unroll
for (int idx = 0; idx < SR_SIZE; idx++) {
	#ifndef DEBUG
	shift_register[idx] = 0.0f;
	#else // DEBUG
	shift_register[idx] = -1.0f;
	#endif // DEBUG
}

// Compute every spatial block:
int block_shift = HALO_BLOCK_WIDTH * y_size + SR_BASE; // Amount of shifts to perform per block
// HALO_BLOCK_WIDTH * y_size to process all the elements in the block
// SR_BASE to initialize the shift register
int total_to_shift = n_blocks * block_shift; // Total amount of shifts to perform
int shift_idx      = 0;                      // Index of current shift

// Indeces:
int x = -RAD;  // x in the block; range: [-RAD, BLOCK_WIDTH)
			   // the range is multiple of VEC_SIZE, to have aligned accesses
int gx = x;    // x in the global matrix
int gy = -RAD; // y in the global matrix; regular range: [0, y_size)
			   // (with initial offset to initialize the shift register)
while (shift_idx < total_to_shift) {
	// Shift the shift register:
	#pragma unroll
	for (int i = 0; i < SR_BASE; i++)
		shift_register[i] = shift_register[i + VEC_SIZE];

	// Read input data from global matrix (vectorized, VEC_SIZE width):
	#pragma unroll
	for (int vec_idx = 0; vec_idx < VEC_SIZE; vec_idx++) {
		// Read coords: The element read is the current "center"
		int vgx_to_read = gx + vec_idx;
		int vgy_to_read = gy;

		#ifndef DEBUG
		shift_register[SR_BASE + vec_idx] = hit(matrixCopy, vgy_to_read, vgx_to_read);
		// Incorrect (out of bounds) accesses will be discarded later
		#else // DEBUG
		if (vgx_to_read < x_size)
			shift_register[SR_BASE + vec_idx] = hit(matrixCopy, vgy_to_read, vgx_to_read);
		else
			shift_register[SR_BASE + vec_idx] = -1.0f;
		#endif // DEBUG
	}

	// Compute the stencil (vectorized, VEC_SIZE width):
	#pragma unroll
	for (int vec_idx = 0; vec_idx < VEC_SIZE; vec_idx++) {
		// Compute coords: The element computed is the current "center"
		int vx  = x + vec_idx;
		int vgx = gx + vec_idx;

		bool shift_register_initialized = gy >= 0;

		// Compute and store stencil result of inner part (skip halos and out of bounds data):
		// (Out of bounds data can only happen in the last, non-full, block,
		// as HALO_BLOCK_WIDTH is multiple of VEC_SIZE)
		bool in_bounds = vx >= 0 && vx < BLOCK_WIDTH && (!MORE_THAN_ONE_BLOCK || vgx < x_size);
		// (in_bounds: We are assuming gy < y_size is true for borders)
		float *sr_center = &shift_register[SR_CENTER + vec_idx];
		// Shift register must be fully shifted here
		if (shift_register_initialized && in_bounds)
			hit(matrix, gy, vgx) = (2.0f * (sr_center[-HALO_BLOCK_WIDTH] +
											sr_center[-1]) +
									sr_center[-2 * HALO_BLOCK_WIDTH] +
									sr_center[-2] +
									.5f * sr_center[-HALO_BLOCK_WIDTH - 1]) /
								   6.5f;
	}

	// Advance indeces:
	shift_idx += VEC_SIZE;

	gx += VEC_SIZE;
	x = (x + VEC_SIZE + RAD) % HALO_BLOCK_WIDTH - RAD;
	if (x == -RAD) { // End of row, move to next line
		gy++;
		if (MORE_THAN_ONE_BLOCK && gy == y_size) {
			// End of block, move to next block
			gy = -RAD; // Offset to initialize the shift register
			gx += BLOCK_WIDTH;
		}
		// Reset global x coord:
		gx -= HALO_BLOCK_WIDTH;
	}
}

// Undefine file-exclusive macros:
#undef MORE_THAN_ONE_BLOCK
