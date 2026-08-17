/**
 * @file epsilod_io.c
 * @brief Epsilod: Environment variables handling
 *
 * @copyright This software is part of the EPSILOD project by Trasgo Group, UVa.
 * The relevant license, warranty and copyright notice is available in the EPSILOD project repository.
 */

#include "epsilod_env.h"
#include "epsilod_log.h"

#include <ctype.h>

const char *io_options[] = {"none", "array", "tile", NULL};

void epsilod_env_load() {
	epsilod_exp_mode();
	epsilod_log_tiles();
	epsilod_log_threads();
	epsilod_warmup();
	epsilod_align();
	mpi_dev_aware();
	epsilod_comm_method();
	comms_contiguous_buffers();
	epsilod_read_input();
	epsilod_write_input();
	epsilod_write_output();
}

bool epsilod_exp_mode() {
	static int val = -1;
	if (val != -1)
		return val;

	val = hit_envNoYes("CTRL_EXAMPLES_EXP_MODE");
	return val;
}

bool epsilod_log_tiles() {
	static int val = -1;
	if (val != -1)
		return val;

	val = hit_envNoYes("EPSILOD_LOG_TILES");
	return val;
}

bool epsilod_log_threads() {
	static int val = -1;
	if (val != -1)
		return val;

	val = hit_envNoYes("EPSILOD_LOG_THREADS");
	return val;
}

bool epsilod_warmup() {
	static int val = -1;
	if (val != -1)
		return val;

	val = hit_envNoYes("EPSILOD_WARMUP");
	return val;
}

EpsilodMemAlignMode epsilod_align() {
	static const char *options[] = {"no", "yes", "threads", NULL};
	static int         val       = -1;
	if (val != -1)
		return val;

	val = hit_envOptions("EPSILOD_ALIGN", options);
	return val;
}

int get_partition_dim(int dims, const char *partition_str, int start) {
	const char *part_arg = &partition_str[1];
	char       *err;
	int         dim = (int)strtol(part_arg, &err, 10);
	if (err == part_arg || dim < start || dim > dims - 1 + start) {
		fprintf(stderr, "\nError in EPSILOD_PARTITION enviroment string: Dimension should be in the range [%d:%d]. String: %s \n\n", start, dims - 1 + start, partition_str);
		MPI_Abort(MPI_COMM_WORLD, MPI_ERR_OTHER);
		exit(EXIT_FAILURE);
	}
	return dim;
}

PartitionInfo get_partition_info(int dims) {
	PartitionInfo info = {
		.type = EPSILOD_PARTITION_MULTI_DIM,
		.dims = dims,
		.dim  = -1};
	char *partition_str = getenv("EPSILOD_PARTITION");
	if (partition_str != NULL) {
		if (strlen(partition_str) > 2) {
			fprintf(stderr, "\nError in EPSILOD_PARTITION enviroment string: More than two characters. String: %s\n\n", partition_str);
			MPI_Abort(MPI_COMM_WORLD, MPI_ERR_OTHER);
			exit(EXIT_FAILURE);
		}
		switch (tolower(partition_str[0])) {
			/* Multidim partition */
			case 'm':
				info.type = EPSILOD_PARTITION_MULTI_DIM;
				// Number of dimensiones in the partition, default all
				if (partition_str[1] != '\0') {
					info.dims = get_partition_dim(dims, partition_str, 1);
				}
				break;
			/* Weighted partition in a single dimension */
			case 'w':
				info.type = EPSILOD_PARTITION_WEIGHTED;
				info.dims = 1;
				info.dim  = get_partition_dim(dims, partition_str, 0);
				break;
			/* Regular partition in a single dimension */
			case 's':
				info.type = EPSILOD_PARTITION_SINGLE_DIM;
				info.dims = 1;
				info.dim  = get_partition_dim(dims, partition_str, 0);
				break;
			/* Regular partition in all dimension except one */
			case 'n':
				info.type = EPSILOD_PARTITION_NOT_DIM;
				info.dims = dims - 1;
				info.dim  = get_partition_dim(dims, partition_str, 0);
				break;
			default:
				fprintf(stderr, "\nError in EPSILOD_PARTITION enviroment string: Unknown partition type. String: %s\n\n", partition_str);
				MPI_Abort(MPI_COMM_WORLD, MPI_ERR_OTHER);
				exit(EXIT_FAILURE);
		}
	}
	return info;
}

bool mpi_dev_aware() {
	// Read env: use CUDA/HIP MPI aware
	static int mpi_dev_aware = -1;
	if (mpi_dev_aware != -1) return mpi_dev_aware;

	mpi_dev_aware = hit_envNoYes("EPSILOD_MPI_DEV_AWARE");
	print_once("Epsilod Using Device-Aware MPI: %c\n", (mpi_dev_aware) ? 'y' : 'n');
	if (mpi_dev_aware)
		print_once(BOLD_TEXT "NOTE:" REGULAR_TEXT "Device-Aware MPI only works if it is suported and activated in the MPI layer\n");
	return mpi_dev_aware;
}

EpsilodCommMethod epsilod_comm_method() {
	static int val = -1;
	if (val != -1)
		return val;

	const char *options[] = {"host_waitany", "host_waitany_recvfirst", "host_waitall"};
	val                   = hit_envOptions("EPSILOD_COMM_METHOD", options);
	switch (val) {
		case 0:
			val = HOST_WAITANY;
			break;
		case 1:
			val = HOST_WAITANY_RECVFIRST;
			break;
		case 2:
			val = HOST_WAITALL;
			break;
	}
	return val;
}

bool comms_contiguous_buffers() {
	static int val = -1;
	if (val != -1)
		return val;

	val = hit_envYesNo("EPSILOD_COMMS_CONTIGUOUS_BUFFERS");
	return val;
}

IOTileMode epsilod_read_input() {
	static int val = -1;
	if (val != -1)
		return (IOTileMode)val;

	val = hit_envOptions("EPSILOD_READ_INPUT", io_options);
	return val;
}

IOTileMode epsilod_write_input() {
	static int val = -1;
	if (val != -1)
		return (IOTileMode)val;

	val = hit_envOptions("EPSILOD_WRITE_INPUT", io_options);
	return val;
}

IOTileMode epsilod_write_output() {
	static int val = -1;
	if (val != -1)
		return (IOTileMode)val;

	val = hit_envOptions("EPSILOD_WRITE_OUTPUT", io_options);
	return val;
}
