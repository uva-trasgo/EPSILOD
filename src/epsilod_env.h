/**
 * @file epsilod_io.c
 * @brief Epsilod: Environment variables handling
 *
 * @copyright This software is part of the EPSILOD project by Trasgo Group, UVa.
 * The relevant license, warranty and copyright notice is available in the EPSILOD project repository.
 */

#ifndef _EPSILOD_ENV_H_
#define _EPSILOD_ENV_H_

#include "epsilod_structs.h"

/**
 * @brief Populates the static storage used for EPSILOD's environment
 */
void epsilod_env_load();

/**
 * @brief Whether EPSILOD should run in experimentation mode.
 * This affects the verbosity of output and its format.
 *
 * @return true if experimentation mode is active, false otherwise.
 */
bool epsilod_exp_mode();

/**
 * @brief Whether EPSILOD should print tile debug information.
 *
 * @return true if tile debug information is expected, false otherwise.
 */
bool epsilod_log_tiles();

/**
 * @brief Whether EPSILOD should print threads and characterizations debug information.
 *
 * @return true if tile debug information is expected, false otherwise.
 */
bool epsilod_log_threads();

/**
 * @brief Whether EPSILOD should perform warmup iterations.
 * This allows data communication systems (such as MPI) to initialize structures that use a lazy aproach.
 * In turn, the overhead of this aproach is not measured in compute times, which eases measurement analysis.
 *
 * @return true if warmup iterations should be performed, false otherwise.
 */
bool epsilod_warmup();

/**
 * @brief Get EPSILOD's memory alignment mode.
 * The mode is obtained from the EPSILOD_ALIGN environment variable.
 * The posible values are: "no", "yes" and "threads". The default value is "no".
 * See EpsilodMemAlignMode for the corresponding memory alignment modes.
 * @return A EpsilodMemAlignMode enum value.
 */
EpsilodMemAlignMode epsilod_align();

/**
 * @brief Gets data necesary to perform the selected partition scheme on the global tile.
 * @param dims The number of dimensions of the domain.
 * @return Partition data.
 */
PartitionInfo get_partition_info(int dims);

/**
 * @brief Whether EPSILOD should use device-aware MPI for communications.
 * @return true if device-aware MPI should be used, false otherwise.
 */
bool mpi_dev_aware();

/**
 * @brief Get the communication method to be used.
 * This method can be specified by the EPSILOD_COMM_METHOD enviroment variable.
 * Defaults to \e HOST_WAITANY
 * @return communication method
 */
EpsilodCommMethod epsilod_comm_method();

/**
 * @brief Whether EPSILOD should use separate contiguous buffers for communications.
 * @return true if separate buffers should be used, false otherwise.
 */
bool comms_contiguous_buffers();

/**
 * @brief Whether EPSILOD should read input from a file.
 * @see IOTileMode
 * @return IOTileMode, EPSILOD_FILE_NONE if input is not read.
 */
IOTileMode epsilod_read_input();

/**
 * @brief Whether EPSILOD should write initial state to a file.
 * @see IOTileMode
 * @return IOTileMode, EPSILOD_FILE_NONE if input is not written.
 */
IOTileMode epsilod_write_input();

/**
 * @brief Whether EPSILOD should write output to a file.
 * @see IOTileMode
 * @return IOTileMode, EPSILOD_FILE_NONE if output is not written.
 */
IOTileMode epsilod_write_output();

#endif
