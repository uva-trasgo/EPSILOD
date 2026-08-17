/**
 * @file epsilod_io.c
 * @brief Epsilod: Input / output handling
 *
 * @copyright This software is part of the EPSILOD project by Trasgo Group, UVa.
 * The relevant license, warranty and copyright notice is available in the EPSILOD project repository.
 */

#include "epsilod_io.h"
#include "epsilod_env.h"

void epsilod_read_input_default(HitTile(EPSILOD_BASE_TYPE) io_tile, EpsilodCoords global, Epsilod_ext *ext_params) {
	IOTileMode io_read_input = epsilod_read_input();
	if (io_read_input != EPSILOD_FILE_NONE) {
		hit_tileFileReadOptions(&io_tile, "Matrix.in", NULL, HIT_FILE_RUNTIME, HIT_FILE_RUNTIME, io_read_input - 1, HIT_FILE_RUNTIME, HIT_FILE_FLOAT, HIT_FILE_RUNTIME, HIT_FILE_RUNTIME);
	}
}

void epsilod_write_input_default(HitTile(EPSILOD_BASE_TYPE) io_tile, EpsilodCoords global, Epsilod_ext *ext_params) {
	IOTileMode io_write_input = epsilod_write_input();
	if (io_write_input != EPSILOD_FILE_NONE) {
		hit_tileFileWriteOptions(&io_tile, "Matrix.copy", NULL, HIT_FILE_RUNTIME, HIT_FILE_RUNTIME, io_write_input - 1, HIT_FILE_RUNTIME, HIT_FILE_FLOAT, HIT_FILE_RUNTIME, HIT_FILE_RUNTIME);
	}
}

void epsilod_write_output_default(HitTile(EPSILOD_BASE_TYPE) io_tile, Epsilod_ext *ext_params) {
	IOTileMode io_write_output = epsilod_write_output();
	if (io_write_output != EPSILOD_FILE_NONE) {
		hit_tileFileWriteOptions(&io_tile, "Matrix.out", NULL, HIT_FILE_RUNTIME, HIT_FILE_RUNTIME, io_write_output - 1, HIT_FILE_RUNTIME, HIT_FILE_FLOAT, HIT_FILE_RUNTIME, HIT_FILE_RUNTIME);
	}
}
