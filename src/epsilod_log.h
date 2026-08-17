/**
 * @file epsilod_io.c
 * @brief Epsilod: Logging and information printing
 *
 * @copyright This software is part of the EPSILOD project by Trasgo Group, UVa.
 * The relevant license, warranty and copyright notice is available in the EPSILOD project repository.
 */

#ifndef _EPSILOD_LOG_H_
#define _EPSILOD_LOG_H_

#include "epsilod_structs.h"

/* Color text modifiers shortcut */
#define BOLD_TEXT    "\e[1m"
#define REGULAR_TEXT "\e[m"

/**
 * Print by all processes. Disabled if Epsilod's experimentation mode is active.
 * @param format pointer to a null-terminated byte string specifying how to interpret the data
 * @param ... arguments specifying data to print.
 */
void print_all(const char *format, ...);

/**
 * Print by rank 0. Disabled if Epsilod's experimentation mode is active.
 * @param format pointer to a null-terminated byte string specifying how to interpret the data
 * @param ... arguments specifying data to print.
 */
void print_once(const char *format, ...);

/**
 * @brief Gather the given string buffer by rank 0 and print the gathered strings.
 * Inactive processes in the given layout are ignored.
 * @param lay The HitLayout used to gather the strings.
 * @param buffer String buffer.
 * @param buffer_size Buffer size.
 * @param separator A string separator between the gathered buffers.
 */
void print_gather(HitLayout lay, char *buffer, size_t buffer_size, const char *separator);

/**
 * @brief Gather the given string buffer by rank 0, reduce them to unique strings and print them.
 * A prefix is added to each unique string to indicate the ranks it originates from.
 * This prefix is a comma separated list of ranks or rank ranges. Rank ranges are in the form a-b,
 * where \a a and \a b are the starting and ending ranks of the range respectively.
 * @param buffer String buffer.
 * @param buffer_size Buffer size.
 * @param separator A string separator between the gathered buffers.
 */
void print_gather_reduce(char *buffer, size_t buffer_size, const char *separator);

// TODO @davdiez move to hitmap and use in dumpShape
/**
 * Get the string representation of a shape in the given buffer
 *
 * @param[out] buff The output buffer.
 * @param sh \e HitShape A HitShape.
 *
 * @return The number of characters written to the buffer
 */
int shape_to_str(char *buff, HitShape sh);

/**
 * Get the string representation of a tile in the given buffer
 *
 * @param[out] buffer The output buffer.
 * @param p_tile A pointer to the tile.
 *
 * @return The number of characters written to the buffer
 */
int tile_to_str(char *buffer, HitTile(EPSILOD_BASE_TYPE) * p_tile);

/**
 * @brief Get the string representation of a Ctrl_Thread in the given buffer
 * @param buffer The output buffer.
 * @param thread Controllers thread structure
 * @param prefix A string printed before threads data
 *
 * @return The number of characters written to the buffer
 */
int thread_to_str(char *buffer, Ctrl_Thread thread, const char prefix[]);

/**
 * @brief Prints controllers information on main process
 */
void print_ctrl_info();

/**
 * @brief Prints topology information
 * @param topo Topology
 */
void print_topo_info(HitTopology topo);

/**
 * @brief Prints layout information
 * @param lay Layout
 */
void print_lay_info(HitLayout lay);

/**
 * @brief Prints weight information
 * @param weights Weights
 */
void print_weight_info(HitWeights weights);

/**
 * @brief Prints information about local subselection tiles.
 * @param lay The HitLayout used in the stencil computation.
 * @param p_tiles Local subselection tiles.
 */
void log_tiles(HitLayout lay, EpsilodTiles *p_tiles);

/**
 * @brief Prints information about the global coordinates of local subselection tiles.
 * @param g_coords Global coordinates data of local tiles.
 */
void log_global_coords(EpsilodGlobalCoords g_coords);

/**
 * @brief Prints information about the threads structures used for EPSILOD tiles.
 * May be used for threads or characterizations.
 * Omits the threads associated to NULL tiles.
 * @param lay EPSILOD's layout
 * @param header A string that is printed before the rest of the information
 * @param threads EPSILOD threads structures
 * @param p_tiles EPSILOD tiles
 */
void log_threads(HitLayout lay, const char header[], EpsilodThreads threads, EpsilodTiles *p_tiles);

#endif
