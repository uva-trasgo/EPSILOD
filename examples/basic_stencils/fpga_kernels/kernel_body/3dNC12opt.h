/**
 * @file basic_stencils/fpga_kernels/kernel_body/3dNC12opt.h
 * @brief File to be included multiple times inside 3dNC12opt.cl, to avoid repetition of hard-to-debug
 * code.
 *
 * This file expects x_size, y_size and z_size to be defined int variables in the file where it is
 * included.
 * It also expects all the UPPER_SNAKE_CASE symbols to be defined constant preprocessor values.
 *
 * @copyright This software is part of the EPSILOD project by Trasgo Group, UVa.
 * The relevant license, warranty and copyright notice is available in the EPSILOD project repository.
 */

/* Default definitions for macros exclusive to this file: */

// Macro to signal that more than one block in each dimension will be computed,
// manages certain logic
#ifndef MORE_THAN_ONE_Y_BLOCK
#define MORE_THAN_ONE_Y_BLOCK true
#endif // MORE_THAN_ONE_Y_BLOCK

// Macro to access the data arrays
#ifndef HIT
#define HIT(...) hit(__VA_ARGS__)
#endif // HIT

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
// Amount of shifts to perform per block:
int block_shift = HALO_BLOCK_WIDTH_X * HALO_BLOCK_WIDTH_Y * z_size + SR_BASE;
// HALO_BLOCK_WIDTH_X * HALO_BLOCK_WIDTH_Y * z_size to process all the elements in the block
// SR_BASE to initialize the shift register
int total_to_shift = n_blocks * block_shift; // Total amount of shifts to perform
int shift_idx      = 0;                      // Index of current shift

// Indeces:
int x = -RAD;      // x in the block; range: [-RAD, BLOCK_WIDTH_X + RAD),
				   // the range is multiple of VEC_SIZE, to have aligned accesses
int gx = x;        // x in the global matrix
int y  = -RAD;     // y in the block; range: [-RAD, BLOCK_WIDTH_Y + RAD),
int gy = y;        // y in the global matrix
int gz = -2 * RAD; // z in the global matrix (offset to initialize the shift register each block)
				   // range: [-2 * RAD, z_size)
while (shift_idx < total_to_shift) {
	// Shift the shift register:
	#pragma unroll
	for (int i = 0; i < SR_BASE; i++)
		shift_register[i] = shift_register[i + VEC_SIZE];

	// Read input data from global matrix (vectorized, VEC_SIZE width):
	#pragma unroll
	for (int vec_idx = 0; vec_idx < VEC_SIZE; vec_idx++) {
		// Read coords: The element read is the belowmost neighbor of the current center
		int vgx_to_read = gx + vec_idx;
		int vgy_to_read = gy;
		int vgz_to_read = gz + RAD;

		#ifndef DEBUG
		shift_register[SR_BASE + vec_idx] = HIT(matrixCopy, vgz_to_read, vgy_to_read, vgx_to_read);
		// Incorrect (out of bounds) accesses will be discarded later
		#else // DEBUG
		if (vgx_to_read < x_size + RAD && vgy_to_read < y_size + RAD)
			shift_register[SR_BASE + vec_idx] = HIT(matrixCopy, vgz_to_read, vgy_to_read, vgx_to_read);
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

		bool shift_register_initialized = gz >= 0;

		// Compute and store stencil result of inner part (skip halos and out of bounds data):
		// (Out of bounds data can only happen in the last, non-full, block,
		// as HALO_BLOCK_WIDTH_X is multiple of VEC_SIZE)
		bool   x_in_bounds = vx >= 0 && vx < BLOCK_WIDTH_X && vgx < x_size;
		bool   y_in_bounds = y >= 0 && y < BLOCK_WIDTH_Y && (!MORE_THAN_ONE_Y_BLOCK || gy < y_size);
		bool   in_bounds   = x_in_bounds && y_in_bounds;
		float *sr_center   = &shift_register[SR_CENTER + vec_idx];
		// Shift register must be fully shifted here
		if (shift_register_initialized && in_bounds)
			HIT(matrix, gz, gy, vgx) = (sr_center[-2 * HALO_BLOCK_WIDTH_Y * HALO_BLOCK_WIDTH_X] +
										sr_center[+2 * HALO_BLOCK_WIDTH_Y * HALO_BLOCK_WIDTH_X] +
										sr_center[-2 * HALO_BLOCK_WIDTH_X] +
										sr_center[+2 * HALO_BLOCK_WIDTH_X] +
										sr_center[-2] +
										sr_center[+2] +
										6 * (sr_center[-HALO_BLOCK_WIDTH_Y * HALO_BLOCK_WIDTH_X] +
											 sr_center[+HALO_BLOCK_WIDTH_Y * HALO_BLOCK_WIDTH_X] +
											 sr_center[-HALO_BLOCK_WIDTH_X] +
											 sr_center[+HALO_BLOCK_WIDTH_X] +
											 sr_center[-1] +
											 sr_center[+1])) /
									   42;
	}

	// Advance indeces:
	shift_idx += VEC_SIZE;

	gx += VEC_SIZE;
	x = (x + VEC_SIZE + RAD) % HALO_BLOCK_WIDTH_X - RAD;
	if (x == -RAD) {              // End of row
		gx -= HALO_BLOCK_WIDTH_X; // Reset global x coord
		gy++;                     // Move to next line
		y = (y + 1 + RAD) % HALO_BLOCK_WIDTH_Y - RAD;
		if (y == -RAD) {                                     // End of slice
			gy -= HALO_BLOCK_WIDTH_Y;                        // Reset global y coord
			gz++;                                            // Move to next plane
			if (gz == z_size) {                              // End of block
				gz = -2 * RAD;                               // Offset to initialize the shift register
				gx += BLOCK_WIDTH_X;                         // Move to next x block
				if (MORE_THAN_ONE_Y_BLOCK && gx >= x_size) { // End of X blocks
					// (The comparison above is ge and not eq because the real X size of the
					// array is not known and can be non-multiple of HALO_BLOCK_WIDTH_X.)
					gx = -RAD;           // Reset global x coord to the beginning of the row
					gy += BLOCK_WIDTH_Y; // Move to next y block
				}
			}
		}
	}
}

// Undefine file-exclusive macros:
#undef MORE_THAN_ONE_Y_BLOCK
#undef HIT
