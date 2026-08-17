/**
 * @file epsilod_io.c
 * @brief Epsilod: Input / output handling
 *
 * @copyright This software is part of the EPSILOD project by Trasgo Group, UVa.
 * The relevant license, warranty and copyright notice is available in the EPSILOD project repository.
 */

#ifndef _EPSILOD_IO_H_
#define _EPSILOD_IO_H_

#include "epsilod_structs.h"

/**
 * @brief Default method to read EPSILOD's input tile from a file.
 * Uses environement variable EPSILOD_READ_INPUT with posible values: "none", "array" or "tile".
 * @param io_tile Tile to read
 * @param global global coordinates
 * @param ext_params extra parameters
 */
void epsilod_read_input_default(HitTile(EPSILOD_BASE_TYPE) io_tile, EpsilodCoords global, Epsilod_ext *ext_params);

/**
 * @brief Default method to write EPSILOD's initial state tile to a file.
 * Uses environement variable EPSILOD_WRITE_INPUT with posible values: "none", "array" or "tile".
 * @param io_tile Tile to write
 * @param global global coordinates
 * @param ext_params extra parameters
 */
void epsilod_write_input_default(HitTile(EPSILOD_BASE_TYPE) io_tile, EpsilodCoords global, Epsilod_ext *ext_params);

/**
 * @brief Default method to write EPSILOD's output tile to a file.
 * Uses environement variable EPSILOD_WRITE_OUTPUT with posible values: "none", "array" or "tile".
 * @param io_tile Tile to write
 * @param global global coordinates
 * @param ext_params extra parameters
 */
void epsilod_write_output_default(HitTile(EPSILOD_BASE_TYPE) io_tile, Epsilod_ext *ext_params);

#endif
