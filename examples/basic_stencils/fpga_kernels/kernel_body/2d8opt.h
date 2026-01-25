/**
 * @file basic_stencils/fpga_kernels/kernel_body/2d8opt.h
 * @brief File to be included multiple times inside 2d8opt.cl, to avoid repetition of hard-to-debug code.
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
int x = -RAD + COMPUTE_X_OFFSET; // x in the block; regular range: [-RAD, BLOCK_WIDTH + RAD),
								 // (with initial offset to initialize the shift register)
int gx = x;                      // x in the global matrix
int gy = -2 * RAD;               // y in the global matrix; regular range: [0, y_size)
								 // (with initial offset to initialize the shift register)
while (shift_idx < total_to_shift) {
	// Shift the shift register:
	#pragma unroll
	for (int i = 0; i < SR_BASE; i++)
		shift_register[i] = shift_register[i + VEC_SIZE];

	// Read input data from global matrix (vectorized, VEC_SIZE width):
	#pragma unroll
	for (int vec_idx = 0; vec_idx < VEC_SIZE; vec_idx++) {
		// Read coords: The element read is the one southeast of the current center
		int vgx_to_read = gx + vec_idx + RAD;
		int vgy_to_read = gy + RAD;
		// Read coordinates cannot overflow to the next row, as read coordinates are aligned
		// to HALO_BLOCK_WIDTH.
		// It's the compute coordinates the ones that would be disaligned due to the corners.
		// (This disalignment is undone to the read coordinates in the +RAD to vgx.)

		#ifndef DEBUG
		shift_register[SR_BASE + vec_idx] = hit(matrixCopy, vgy_to_read, vgx_to_read);
		// Incorrect (out of bounds) accesses will be discarded later
		#else // DEBUG
		if (vgx_to_read < x_size + RAD)
			shift_register[SR_BASE + vec_idx] = hit(matrixCopy, vgy_to_read, vgx_to_read);
		else
			shift_register[SR_BASE + vec_idx] = -1.0f;
		#endif // DEBUG
	}

	// Compute the stencil (vectorized, VEC_SIZE width):
	#pragma unroll
	for (int vec_idx = 0; vec_idx < VEC_SIZE; vec_idx++) {
		// Compute coords: The element computed is the current center
		int vx  = x + vec_idx;
		int vgx = gx + vec_idx;
		// Disaligned compute coords are handled later (when advancing the indeces), by
		// changing row early, so that all vector compute accesses are to the same row.
		// Incorrect coords due to this early change replace the rightmost halo coords;
		// in both cases they fall out of bounds, and thus are not processed.

		bool shift_register_initialized = gy >= 0;

		// Compute and store stencil result of inner part (skip halos and out of bounds data):
		bool   in_bounds = vx >= 0 && vx < BLOCK_WIDTH && (!MORE_THAN_ONE_BLOCK || vgx < x_size);
		float *sr_center = &shift_register[SR_CENTER + vec_idx];
		if (shift_register_initialized && in_bounds)
			hit(matrix, gy, vgx) = (4 * (sr_center[-HALO_BLOCK_WIDTH] +
										 sr_center[+HALO_BLOCK_WIDTH] +
										 sr_center[-1] +
										 sr_center[+1]) +
									sr_center[-HALO_BLOCK_WIDTH - 1] +
									sr_center[+HALO_BLOCK_WIDTH - 1] +
									sr_center[-HALO_BLOCK_WIDTH + 1] +
									sr_center[+HALO_BLOCK_WIDTH + 1]) /
								   20;
	}

	// Advance indeces:
	shift_idx += VEC_SIZE;

	// Revert compute offset temporally to compute the next coords properly:
	// (If not done, the next read coordinates could be invalid.)
	// (This current logic works with all thought scenarios because the compute offset
	// is always equal to the size of the halo (i.e., non-computed part) at the right, so
	// no valid computation is skipped.)
	gx -= COMPUTE_X_OFFSET;
	x -= COMPUTE_X_OFFSET;

	gx += VEC_SIZE;
	x += VEC_SIZE;
	if (x + RAD >= HALO_BLOCK_WIDTH) { // End of row, move to next line
		gy++;
		x = -RAD;
		if (MORE_THAN_ONE_BLOCK && gy == y_size) {
			// End of block, move to next block
			gy = -2 * RAD; // Offset to initialize the shift register
			gx += BLOCK_WIDTH;
		}
		// Reset global x coord:
		gx -= HALO_BLOCK_WIDTH;
	}

	// Restore offset:
	gx += COMPUTE_X_OFFSET;
	x += COMPUTE_X_OFFSET;
	// The compute coords corresponding to the "compute" of the right halo are substituted
	// by other, invalid coords (-2 * RAD), so their compute is skipped anyway.
}

// Undefine file-exclusive macros:
#undef MORE_THAN_ONE_BLOCK
