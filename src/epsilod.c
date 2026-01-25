/**
 * @file epsilod.c
 * @brief Epsilod: Stencil code: Any dimensions, stencil as a pattern of weights. Data type: float
 *
 * @copyright This software is part of the EPSILOD project by Trasgo Group, UVa.
 * The relevant license, warranty and copyright notice is available in the EPSILOD project repository.
 */

#include <stdio.h>

#include "epsilod.h"
#include "../../examples/Utils/ctrl_print_info.h"

/* B. Generic kernel prototype and wrapper launchers */
#if EPSILOD_IS_FLOAT(EPSILOD_BASE_TYPE)
CTRL_KERNEL_CHAR(updateCell_default_1D, MANUAL, 0, 0, 0);
CTRL_KERNEL_CHAR(updateCell_default_2D, MANUAL, 0, 0, 0);
CTRL_KERNEL_CHAR(updateCell_default_3D, MANUAL, 0, 0, 0);
CTRL_KERNEL_CHAR(updateCell_default_4D, MANUAL, 0, 0, 0);

CTRL_KERNEL_PROTO(updateCell_default_1D, 2,
				  GENERIC, DEFAULT,
				  FPGA, NDRANGE,
				  6,
				  OUT, HitTile(EPSILOD_BASE_TYPE), matrix,
				  IN, HitTile(EPSILOD_BASE_TYPE), matrixCopy,
				  INVAL, EpsilodCoords, global_coords,
				  IN, HitTile(float), weight,
				  INVAL, float, factor,
				  INVAL, Epsilod_ext, ext_params);

void updateCell_default_1D(PCtrl ctrl, Ctrl_Thread threads, Ctrl_Thread blockSize, int stream, HitTile(EPSILOD_BASE_TYPE) mat, HitTile(EPSILOD_BASE_TYPE) copy, EpsilodCoords global, HitTile_float weight, float factor, Epsilod_ext *ext_params) {
	Ctrl_LaunchToStream(ctrl, updateCell_default_1D, threads, blockSize, stream, mat, copy, global, weight, factor, *ext_params);
}

CTRL_KERNEL_PROTO(updateCell_default_2D, 2,
				  GENERIC, DEFAULT,
				  FPGA, NDRANGE,
				  6,
				  OUT, HitTile(EPSILOD_BASE_TYPE), matrix,
				  IN, HitTile(EPSILOD_BASE_TYPE), matrixCopy,
				  INVAL, EpsilodCoords, global_coords,
				  IN, HitTile(float), weight,
				  INVAL, float, factor,
				  INVAL, Epsilod_ext, ext_params);

void updateCell_default_2D(PCtrl ctrl, Ctrl_Thread threads, Ctrl_Thread blockSize, int stream, HitTile(EPSILOD_BASE_TYPE) mat, HitTile(EPSILOD_BASE_TYPE) copy, EpsilodCoords global, HitTile_float weight, float factor, Epsilod_ext *ext_params) {
	Ctrl_LaunchToStream(ctrl, updateCell_default_2D, threads, blockSize, stream, mat, copy, global, weight, factor, *ext_params);
}

CTRL_KERNEL_PROTO(updateCell_default_3D, 2,
				  GENERIC, DEFAULT,
				  FPGA, NDRANGE,
				  6,
				  OUT, HitTile(EPSILOD_BASE_TYPE), matrix,
				  IN, HitTile(EPSILOD_BASE_TYPE), matrixCopy,
				  INVAL, EpsilodCoords, global_coords,
				  IN, HitTile(float), weight,
				  INVAL, float, factor,
				  INVAL, Epsilod_ext, ext_params);

void updateCell_default_3D(PCtrl ctrl, Ctrl_Thread threads, Ctrl_Thread blockSize, int stream, HitTile(EPSILOD_BASE_TYPE) mat, HitTile(EPSILOD_BASE_TYPE) copy, EpsilodCoords global, HitTile_float weight, float factor, Epsilod_ext *ext_params) {
	Ctrl_LaunchToStream(ctrl, updateCell_default_3D, threads, blockSize, stream, mat, copy, global, weight, factor, *ext_params);
}

CTRL_KERNEL_PROTO(updateCell_default_4D, 2,
				  GENERIC, DEFAULT,
				  FPGA, NDRANGE,
				  6,
				  OUT, HitTile(EPSILOD_BASE_TYPE), matrix,
				  IN, HitTile(EPSILOD_BASE_TYPE), matrixCopy,
				  INVAL, EpsilodCoords, global_coords,
				  IN, HitTile(float), weight,
				  INVAL, float, factor,
				  INVAL, Epsilod_ext, ext_params);

void updateCell_default_4D(PCtrl ctrl, Ctrl_Thread threads, Ctrl_Thread blockSize, int stream, HitTile(EPSILOD_BASE_TYPE) mat, HitTile(EPSILOD_BASE_TYPE) copy, EpsilodCoords global, HitTile_float weight, float factor, Epsilod_ext *ext_params) {
	Ctrl_LaunchToStream(ctrl, updateCell_default_4D, threads, blockSize, stream, mat, copy, global, weight, factor, *ext_params);
}
#endif // EPSILOD_BASE_TYPE != float

CTRL_KERNEL_CHAR(epsilod_dev_copy_1d, MANUAL, 0, 0, 0);
CTRL_KERNEL_PROTO(epsilod_dev_copy_1d, 2,
				  GENERIC, DEFAULT,
				  FPGA, NDRANGE,
				  2,
				  IN, HitTile(EPSILOD_BASE_TYPE), matrix,
				  OUT, HitTile(EPSILOD_BASE_TYPE), matrix_out);

CTRL_KERNEL_CHAR(epsilod_dev_copy_2d, MANUAL, 0, 0, 0);
CTRL_KERNEL_PROTO(epsilod_dev_copy_2d, 2,
				  GENERIC, DEFAULT,
				  FPGA, NDRANGE,
				  2,
				  IN, HitTile(EPSILOD_BASE_TYPE), matrix,
				  OUT, HitTile(EPSILOD_BASE_TYPE), matrix_out);

CTRL_KERNEL_CHAR(epsilod_dev_copy_3d, MANUAL, 0, 0, 0);
CTRL_KERNEL_PROTO(epsilod_dev_copy_3d, 2,
				  GENERIC, DEFAULT,
				  FPGA, NDRANGE,
				  2,
				  IN, HitTile(EPSILOD_BASE_TYPE), matrix,
				  OUT, HitTile(EPSILOD_BASE_TYPE), matrix_out);

// TODO epsilod_dev_copy_4d

/* D. False initialization of selections to avoid non-initialized warnings */
CTRL_KERNEL_CHAR(epsilod_dev_touch, MANUAL, 0, 0, 0);
CTRL_KERNEL_PROTO(epsilod_dev_touch, 2,
				  GENERIC, DEFAULT,
				  FPGA, TASK,
				  1,
				  OUT, HitTile(EPSILOD_BASE_TYPE), matrix);

CTRL_HOST_TASK(epsilod_host_touch, HitTile(EPSILOD_BASE_TYPE) matrix) { ; }
CTRL_HOST_TASK_PROTO(epsilod_host_touch, 1, OUT, HitTile(EPSILOD_BASE_TYPE), matrix);

/* E. Output host-task wrapper */
CTRL_HOST_TASK(Ctrl_Write_Output, outputDataFunction f_output, HitTile(EPSILOD_BASE_TYPE) matrix, Epsilod_ext *ext_params) {
	f_output(matrix, ext_params);
}

CTRL_HOST_TASK_PROTO(Ctrl_Write_Output, 3,
					 INVAL, outputDataFunction, f_output,
					 IN, HitTile(EPSILOD_BASE_TYPE), matrix,
					 INVAL, Epsilod_ext *, ext_params);

/* F. Stencil pattern transference host-task */
CTRL_HOST_TASK(Ctrl_Copy_Stencil, HitTile_float stencil, float *stencil_data) {
	for (int i = 0; i < hit_tileCard(stencil); i++)
		hit(stencil, i) = stencil_data[i];
}

CTRL_HOST_TASK_PROTO(Ctrl_Copy_Stencil, 2,
					 OUT, HitTile_float, stencil,
					 INVAL, float *, stencil_data);

/* H. Experimentation: global clocks */
HitClock ctrl_clock;
HitClock init_clock;
HitClock loop_clock;
HitClock iter_clock;
HitClock redistribute_clock;
HitClock commClock;

void epsilod_print_usage() {
	if (hit_Rank == 0) {
		fprintf(stderr, "\nEPSILOD environment variables:\n");
		fprintf(stderr, "\tEPSILOD_MPI_DEV_AWARE=y|n    Activate the use of CUDA/HIP aware MPI communications\n");
		fprintf(stderr, "\tEPSILOD_PARTITION=m          Regular blocks of similar sizes on a multidimensional grid topology with the matrix dimensions\n");
		fprintf(stderr, "\tEPSILOD_PARTITION=m<n_dims>  Regular blocks of similar sizes on the first <n_dims> dimensions\n");
		fprintf(stderr, "\tEPSILOD_PARTITION=s<dim>     Regular blocks of similar sizes on a single dimension topology\n");
		fprintf(stderr, "\tEPSILOD_PARTITION=w<dim>     Weigthed block distribution in the single chosen dimension.\n");
		fprintf(stderr, "\t                             Processes weigths are specified in the device selection configuratuion file.\n");
		fprintf(stderr, "\t" BOLD_TEXT "NOTE:" REGULAR_TEXT " The default behaviour corresponds to s0.\n");
		fprintf(stderr, "\tEPSILOD_ALB_HEUR=none        Rebalancing deactivated.\n");
		fprintf(stderr, "\tEPSILOD_ALB_HEUR=NextALB     Try to estimate in which iteration will a new rebalancing be needed.\n");
		fprintf(stderr, "\tEPSILOD_ALB_HEUR=ConstIters  Rebalance after a constant number of iterations.\n");
		fprintf(stderr, "\tEPSILOD_ALB_HEUR=ExpIters    Rebalance after a exponentially increasing number of iterations.\n");
		fprintf(stderr, "\tEPSILOD_ALB_HEUR=DoubleIters Rebalance after an amount of iterations that doubles each time number of iterations.\n");
		fprintf(stderr, "\t" BOLD_TEXT "NOTE:" REGULAR_TEXT " The default behaviour corresponds to none. Anything else requires w partition.\n");
	}
}

/**
 * @brief Print clocks. Format according to Epsilod's experimentation mode
 */
void print_clock_info() {
	if (epsilod_exp_mode()) {
		if (hit_Rank == 0)
			printf("%lf, %lf, %lf, %lf\n", ctrl_clock.max, init_clock.max, loop_clock.max, commClock.max);
	} else {
		hit_clockPrintMax(ctrl_clock);
		hit_clockPrintMax(init_clock);
		hit_clockPrintMax(loop_clock);
		hit_clockPrintMax(redistribute_clock);
		hit_clockPrintMax(commClock);
	}
	fflush(stdout);
}

/**
 * @brief Clock reduction.
 * @param layout The layout
 */
void reduceClocks(HitLayout layout) {
	hit_clockReduce(layout, ctrl_clock);
	hit_clockReduce(layout, init_clock);
	hit_clockReduce(layout, loop_clock);
	hit_clockReduce(layout, redistribute_clock);
	hit_clockReduce(layout, commClock);
}

/**
 * @brief Mark tiles as valid to avoid warnings due to selections status not being handled by ctrl.
 * @param comm Controller object
 * @param threads_touch Dummy thread space for empty kernels
 * @param blocksize_touch Dummy blocksize for empty kernels
 * @param tiles Tiles to mark
 * @param copy_tiles Ancillary tiles to mark
 * @param comm_args Used to check what borders are active.
 *
 * @todo could we avoid \p comm_args by checking if the tiles/shapes are valid?
 */
void markTiles(PCtrl comm, Ctrl_Thread threads_touch, Ctrl_Thread blocksize_touch, EpsilodTiles *tiles, EpsilodTiles *copy_tiles, EpsilodCommArgs *comm_args) {
	int dims = hit_tileDims(tiles->mat);

	for (int i = 0; i < dims; i++) {
		for (int j = 0; j < 2; j++) {
			if (validShape(tiles->border_out_dev[i][j].shape)) {
				Ctrl_Launch(comm, epsilod_dev_touch, threads_touch, blocksize_touch, tiles->border_out_dev[i][j]);
				Ctrl_Launch(comm, epsilod_dev_touch, threads_touch, blocksize_touch, copy_tiles->border_out_dev[i][j]);
			}
		}
	}
	if (validShape(tiles->inner.shape)) {
		Ctrl_Launch(comm, epsilod_dev_touch, threads_touch, blocksize_touch, tiles->inner);
		Ctrl_Launch(comm, epsilod_dev_touch, threads_touch, blocksize_touch, copy_tiles->inner);
	}
	for (int i = 0; i < epsilod_num_borders(dims); i++) {
		// Skip empty borders
		if (!comm_args->border_in_active[i]) continue;
		Ctrl_Launch(comm, epsilod_dev_touch, threads_touch, blocksize_touch, tiles->border_in[i]);
	}
}

typedef void (*CommsFunction)(PCtrl, EpsilodTiles *, EpsilodCommArgs *);
typedef void (*CommsInnerFunction)(PCtrl, EpsilodTiles *, EpsilodCommArgs *);

/**
 * @brief Perform communications.
 * This includes host-device communications (if mpi aware is disabled) and interprocess communications
 * @param comm Controller object
 * @param tiles Tiles to communicate
 * @param args Arguments for communications
 */
CommsFunction do_comms;

/**
 * Communication method dependent implementation
 */
CommsInnerFunction do_comms_inner;

/**
 * @brief Perform communications with host staging buffers.
 * This includes host-device communications and interprocess communications
 * @param comm Controller object
 * @param tiles Tiles to communicate
 * @param args Arguments for communications
 */
void do_comms_host(PCtrl comm, EpsilodTiles *tiles, EpsilodCommArgs *args) {
	int dims        = hit_tileDims(tiles->mat);
	int num_borders = epsilod_num_borders(dims);

	for (int i = 0; i < dims; i++) {
		for (int j = 0; j < 2; j++) {
			if (validShape(tiles->border_out_dev[i][j].shape)) {
				Ctrl_MoveFrom(comm, tiles->border_out_dev[i][j]);
			}
		}
	}
	for (int i = 0; i < dims; i++) {
		for (int j = 0; j < 2; j++) {
			if (validShape(tiles->border_out_dev[i][j].shape)) {
				Ctrl_WaitTile(comm, tiles->border_out_dev[i][j]);
			}
		}
	}

	hit_clockStart(commClock);
	do_comms_inner(comm, tiles, args);
	for (int i = 0; i < num_borders; i++) {
		// Skip empty borders
		if (!args->border_in_active[i]) continue;
		Ctrl_WaitTile(comm, tiles->border_in[i]);
	}

	hit_clockStop(commClock);
}

/**
 * @brief Inner implementation of host staging communications.
 * After each interprocess transfer is completed, its corresponding HtoD transfer is performed.
 */
static inline void do_comms_host_inner_commany(PCtrl comm, EpsilodTiles *tiles, EpsilodCommArgs *args) {
	hit_patternStartAsync(tiles->neighSync);
	for (int endComm = hit_patternStepAsync(tiles->neighSync);
		 endComm != HIT_PAT_END;
		 endComm = hit_patternStepAsync(tiles->neighSync)) {

		// Skip sends
		if (endComm % 2 == 0) continue;

		// Start move-to for recv
		int border = args->index_comm_border[endComm / 2];

		Ctrl_HostTask(epsilod_host_touch, tiles->border_in[border]);
		Ctrl_MoveTo(comm, tiles->border_in[border]);
	}
}

/**
 * @brief Inner implementation of host staging communications.
 * After each interprocess transfer is completed, its corresponding HtoD transfer is performed.
 * Data recvs are performed first.
 */
static inline void do_comms_host_inner_commanyrecvfirst(PCtrl comm, EpsilodTiles *tiles, EpsilodCommArgs *args) {
	hit_patternStartAsync(tiles->neighSync);
	for (int endComm = hit_patternStepAsyncRecv(tiles->neighSync);
		 endComm != HIT_PAT_END;
		 endComm = hit_patternStepAsyncRecv(tiles->neighSync)) {

		// Start move-to for recv
		int border = args->index_comm_border[endComm];

		Ctrl_HostTask(epsilod_host_touch, tiles->border_in[border]);
		Ctrl_MoveTo(comm, tiles->border_in[border]);
	}
	hit_patternEndAsync(tiles->neighSync);
}

/**
 * @brief Inner implementation of host staging communications.
 * After all interprocess transfers are completed, all HtoD transfers are performed.
 */
static inline void do_comms_host_inner_commall(PCtrl comm, EpsilodTiles *tiles, EpsilodCommArgs *args) {
	hit_patternDo((tiles->neighSync));

	int dims        = hit_tileDims(tiles->mat);
	int num_borders = epsilod_num_borders(dims);
	for (int i = 0; i < num_borders; i++) {
		// Skip empty borders
		if (!args->border_in_active[i]) continue;
		Ctrl_HostTask(epsilod_host_touch, tiles->border_in[i]);
		Ctrl_MoveTo(comm, tiles->border_in[i]);
	}
}

/**
 * @brief Perform interprocess communications with device buffers.
 * @param comm Controller object
 * @param tiles Tiles to communicate
 * @param args Arguments for communications
 */
void do_comms_device(PCtrl comm, EpsilodTiles *tiles, EpsilodCommArgs *args) {
	hit_patternDo((tiles->neighSync));
}

/**
 * @brief Get the communication method to be used.
 * This method can be specified by the EPSILOD_COMM_METHOD enviroment variable.
 * Defaults to \e HOST_WAITANY
 * @return communication method
 */
EpsilodCommMethod epsilod_comm_method() {
	static int val = -1;
	if (val != -1)
		return val;

	const char *options[] = {"host_waitany", "host_waitany_recvfirst", "host_waitall"};
	val                   = hit_envOptions("EPSILOD_COMM_METHOD", options);
	switch (val) {
		case 0:
			val = HOST_WAITANY;
		case 1:
			val = HOST_WAITANY_RECVFIRST;
		case 2:
			val = HOST_WAITALL;
	}
	return val;
}

/**
 * @brief Sets the communication method to be used
 */
void setup_comm_method() {
	if (mpi_dev_aware())
		do_comms = do_comms_device;
	else {
		do_comms = do_comms_host;
		switch (epsilod_comm_method()) {
			case HOST_WAITANY:
				do_comms_inner = do_comms_host_inner_commany;
				break;
			case HOST_WAITANY_RECVFIRST:
				do_comms_inner = do_comms_host_inner_commanyrecvfirst;
				break;
			case HOST_WAITALL:
				do_comms_inner = do_comms_host_inner_commall;
				break;
		}
	}
}

/**
 * @brief Launch stencil computation kernels
 * @param comm Controller object
 * @param f_updateCell Stencil kernel wrapper function
 * @param tiles Tiles to update (write)
 * @param tiles_copy Ancillary tiles to read
 * @param threads Thread spaces for kernels
 * @param chars Blocksizes for kernels
 * @param coords Global coordinates information
 * @param stencil Stencil tile
 * @param factor Divisor factor
 * @param ext_params Extra parameters. Defined by the user
 */
void compute(PCtrl comm, stencilDeviceFunction f_updateCell,
			 EpsilodTiles tiles, EpsilodTiles tiles_copy,
			 EpsilodThreads threads, EpsilodThreads chars,
			 EpsilodGlobalCoords coords,
			 HitTile_float stencil, float factor,
			 Epsilod_ext *ext_params) {

	int dims = hit_tileDims(tiles.mat);

	// Compute borders
	for (int i = 0; i < dims; i++) {
		for (int j = 0; j < 2; j++) {
			if (validShape(tiles.border_out_dev[i][j].shape) && validShape(tiles_copy.border_out_dev[i][j].shape)) {
				f_updateCell(comm, threads.border_out_dev[i][j], chars.border_out_dev[i][j], 2 * i + j, tiles.border_out_dev[i][j], tiles_copy.border_out_dev[i][j], coords.border_out_dev[i][j], stencil, factor, ext_params);
			}
		}
	}

	// Sync borders before inner
	for (int i = 0; i < dims; i++) {
		for (int j = 0; j < 2; j++) {
			if (validShape(tiles.border_out_dev[i][j].shape) && validShape(tiles_copy.border_out_dev[i][j].shape)) {
				Ctrl_WaitTile(comm, tiles.border_out_dev[i][j]);
			}
		}
	}

	// Compute inner
	if (validShape(tiles.inner.shape) && validShape(tiles_copy.inner.shape)) {
		f_updateCell(comm, threads.inner, chars.inner, 0, tiles.inner, tiles_copy.inner, coords.inner, stencil, factor, ext_params);
	}
}

/**
 * @brief Create the global matrix encompassing the whole domain.
 * @param dims Number of dimensions
 * @param sizes Size for each dimension
 * @return Global matrix
 */
HitTile(EPSILOD_BASE_TYPE) create_global_mat(int dims, HitInd sizes[dims]) {
	HitShape shp = HIT_SHAPE_NULL;
	hit_shapeDimsSet(shp, dims);
	for (int i = 0; i < dims; i++) {
		hit_shapeSig(shp, i) = hit_sig(0, sizes[i] - 1, 1);
	}
	return Ctrl_Domain(EPSILOD_BASE_TYPE, shp);
}

/**
 * @brief Prints controllers information on main process
 */
void print_ctrl_info() {
	size_t      stdout_buffer_size = 100 * 1024;
	char        stdout_buffer[stdout_buffer_size];
	char       *p_stdout_buffer = stdout_buffer;
	const char *sep             = epsilod_exp_mode() ? "&" : "\n\n";
	if (!epsilod_exp_mode())
		p_stdout_buffer += sprintf(p_stdout_buffer, "MPI_Rank[%d]", hit_Rank);
	p_stdout_buffer += Ctrl_SPrintInfo(p_stdout_buffer, epsilod_exp_mode());
	print_gather(stdout_buffer, stdout_buffer_size, sep);
	fflush(stdout);
}

/**
 * @brief Debug topology information dump
 * @param topo Topology
 */
void print_topo_info(HitTopology topo) {
	#ifdef _EPSILOD_TOPO_INFO_
	if (hit_Rank == 0) {
		char  buffer[1024];
		char *p_buffer = buffer;
		p_buffer += sprintf(p_buffer, "\nTOPOLOGY: ");
		for (int i = 0; i < hit_topDims(topo); i++) {
			p_buffer += sprintf(p_buffer, "%dx", hit_topDimCard(topo, i));
		}
		p_buffer--;
		p_buffer += sprintf(p_buffer, "\n");
		printf("%s", buffer);
		fflush(stdout);
	}
	#endif // _EPSILOD_TOPO_INFO_
}

/**
 * @brief Debug layout information dump
 * @param lay Layout
 */
void print_lay_info(HitLayout lay) {
	#ifdef _EPSILOD_LAY_INFO_
	size_t buff_sz  = 100 * 1024;
	char  *buffer   = (char *)malloc(buff_sz * sizeof(char));
	char  *p_buffer = buffer;

	p_buffer += sprintf(p_buffer, "[%d] lay->orig: ", hit_Rank);
	p_buffer += shape_to_str(p_buffer, lay.origShape);
	p_buffer += sprintf(p_buffer, "\n");

	p_buffer += sprintf(p_buffer, "[%d] lay->shape: ", hit_Rank);
	p_buffer += shape_to_str(p_buffer, lay.shape);
	p_buffer += sprintf(p_buffer, "\n");

	print_gather(buffer, buff_sz, "\n");
	free(buffer);
	#endif // _EPSILOD_LAY_INFO_
}

/**
 * @brief Debug weight information dump
 * @param weights Weights
 */
void print_weight_info(HitWeights weights) {
	#ifdef _EPSILOD_WEIGHTS_INFO_
	if (hit_Rank == 0) {
		printf("\nPartition weights = {");
		for (int i = 0; i < weights.num_procs; i++)
			printf(" %f,", weights.ratios[i]);
		printf("\b }\n");
		fflush(stdout);
	}
	#endif // _EPSILOD_WEIGHTS_INFO_
}

/**
 * @brief Check for too many processes in partition
 * @param lay Layout
 * @param borders Stencil border sizes
 */
void check_partition_data(HitLayout lay, EpsilodBorders borders) {
	HitTile_epsilod_error local  = hitTile_epsilod_error(hitShapeSize(1));
	HitTile_epsilod_error global = hitTile_epsilod_error(hitShapeSize(1));
	hit(local, 0)                = 0;
	hit(global, 0)               = 0;

	for (int i = 0; i < hit_layNumDims(lay); i++) {
		if (hit_shapeSigCard(lay.shape, i) < borders.low[i] ||
			hit_shapeSigCard(lay.shape, i) < borders.high[i]) {
			hit(local, 0) = i + 1;
		}
	}

	HitOp op_max;
	hit_comOp(hit_comOpMaxInt, op_max);
	hit_comDoOnce(hit_comReduce(lay, HIT_RANKS_NULL, &local, &global, HIT_INT, op_max));
	hit_comOpFree(op_max);

	if (hit(global, 0) && hit_Rank == 0) {
		fprintf(stderr, "\nError: Not enough data after partition, too many processes in topology axis: %d\n\n", hit(global, 0) - 1);
		MPI_Abort(MPI_COMM_WORLD, MPI_ERR_TOPOLOGY);
		exit(EXIT_FAILURE);
	}
}

/**
 * @brief Create processes topology
 * @param info Partition information to create the topology
 * @return The topology
 */
HitTopology get_topology(PartitionInfo info) {
	HitTopology topo;
	switch (info.type) {
		case EPSILOD_PARTITION_MULTI_DIM:
		case EPSILOD_PARTITION_NOT_DIM:
			topo = hit_topology(plug_topArray, info.dims);
			break;
		default:
			topo = hit_topology(plug_topPlain);
			break;
	}
	print_topo_info(topo);
	return topo;
}

/**
 * @brief Create data partition layout
 * @param shp_global Shape of the global partition matrix
 * @param borders Stencil border sizes
 * @return The layout
 */
HitLayout get_layout(HitShape shp_global, EpsilodBorders borders) {
	// Shape to distribute computation (without borders)
	HitShape shp_inner = shp_global;
	for (int i = 0; i < hit_shapeDims(shp_inner); i++) {
		shp_inner = hit_shapeTransform(shp_inner, i, HIT_SHAPE_BEGIN, +borders.low[i]);
		shp_inner = hit_shapeTransform(shp_inner, i, HIT_SHAPE_END, -borders.high[i]);
	}

	// Select and build partition/distribution
	PartitionInfo info = get_partition_info(hit_shapeDims(shp_global));

	HitTopology topo = get_topology(info);
	HitLayout   lay;
	switch (info.type) {
		case EPSILOD_PARTITION_MULTI_DIM:
			lay = hit_layout_freeTopo(plug_layBlocks, topo, shp_inner);
			break;
		case EPSILOD_PARTITION_WEIGHTED:
			// Weighted distribution
			lay = hit_layout_freeTopo(plug_layDimWeighted_Blocks, topo, shp_inner, info.dim, Ctrl_ConfigWeights());
			break;
		case EPSILOD_PARTITION_SINGLE_DIM:
			// Regular distribution on a dimension
			lay = hit_layout_freeTopo(plug_layDimBlocks, topo, shp_inner, info.dim);
			break;
		default:
			fprintf(stderr, "Error: EPSILOD partition type not yet implemented\n");
			MPI_Abort(MPI_COMM_WORLD, MPI_ERR_TOPOLOGY);
			exit(EXIT_FAILURE);
			break;
	}

	print_weight_info(Ctrl_ConfigWeights());

	print_lay_info(lay);

	// TODO @seralpa can this be done by innactive processes too?
	if (hit_layImActive(lay)) check_partition_data(lay, borders);
	return lay;
}

/**
 * @brief Create the MPI type to be used for communications
 * @return Type to use for communications
 */
HitType create_comm_type() {
	#if CTRL_COUNTPARAM(EPSILOD_BASE_TYPE_COMPOUND) < 2
	return hit_comTranslateType(EPSILOD_BASE_TYPE);
	#elif CTRL_COUNTPARAM(EPSILOD_BASE_TYPE_COMPOUND) == 2
	HitType HIT_CELL;
	HitType type  = hit_comTranslateType(EPSILOD_GET_COMPOUND_TYPE(EPSILOD_BASE_TYPE_COMPOUND));
	int     count = EPSILOD_GET_COMPOUND_COUNT(EPSILOD_BASE_TYPE_COMPOUND);
	hit_comTypeArray(&HIT_CELL, type, count);
	return HIT_CELL;
	#else // CTRL_COUNTPARAM(EPSILOD_BASE_TYPE_COMPOUND) > 2
	#error "EPSILOD_TYPE_COMPOUND_<type> should contain two arguments separated by comma: <type>, <count>"
	#endif // CTRL_COUNTPARAM(EPSILOD_BASE_TYPE_COMPOUND)
}

void stencilComputation(
	HitInd                 sizes[],
	HitShape               stencilShape,
	float                  stencilData[],
	float                  factor,
	int                    numIterations,
	initDataFunction       f_init,
	initDataDeviceFunction f_dev_init,
	stencilDeviceFunction  f_init_copy,
	stencilDeviceFunction  f_updateCell,
	outputDataFunction     f_output,
	Epsilod_ext           *ext_params_arg,
	char                  *device_selection_file) {

	HitClock comp_clock;
	hit_clockStart(comp_clock);

	int dims = hit_shapeDims(stencilShape);

	// Check if generic kernel has been chosen
	if (f_updateCell == NULL) {
		#if !EPSILOD_IS_FLOAT(EPSILOD_BASE_TYPE)
		fprintf(stderr, "[EPSILOD ERROR] Generic update kernel is only defined for float base type.\n\tFor other types a custom update kernel should be provided as argument.\n");
		fflush(stderr);
		exit(EXIT_FAILURE);
		#else  // float
		switch (dims) {
			case 1: f_updateCell = updateCell_default_1D; break;
			case 2: f_updateCell = updateCell_default_2D; break;
			case 3: f_updateCell = updateCell_default_3D; break;
			case 4: f_updateCell = updateCell_default_4D; break;
			default:
				fprintf(stderr, "[EPSILOD ERROR] Stencil with invalid number of dimensions. EPSILOD only supports up to 4D stencils.\n");
				fflush(stderr);
				exit(EXIT_FAILURE);
		}
		#endif // float
	}

	// Hitmap comm. type
	HitType HIT_CELL = create_comm_type();
	setup_comm_method();

	// Initialize device Controllers
	__ctrl_block__(device_selection_file) {
		// init clocks
		hit_clockSynchronizeAll();
		hit_clockStart(ctrl_clock);
		hit_clockStart(init_clock);

		PCtrl comm = Ctrl_Get(0);

		print_ctrl_info();

		Ctrl_SetDependanceMode(comm, CTRL_MODE_EXPLICIT);

		/* 0. Build the stencil tile */
		HitTile_float stencil = Ctrl_DomainAlloc(comm, float, stencilShape);
		Ctrl_HostTask(Ctrl_Copy_Stencil, stencil, stencilData);
		Ctrl_MoveTo(comm, stencil);
		Ctrl_WaitTile(comm, stencil);

		HitTile(EPSILOD_BASE_TYPE) globalMat = create_global_mat(dims, sizes);

		/* 2. Shortcuts for border sizes */
		EpsilodBorders borders;
		for (int i = 0; i < dims; i++) {
			borders.low[i]  = -hit_tileDimBegin(stencil, i);
			borders.high[i] = hit_tileDimEnd(stencil, i);
		}

		/* 3.2. Build distributed shape */
		HitLayout lay = get_layout(globalMat.shape, borders);

		/* 4. Active processes */
		if (hit_layImActive(lay)) {

			// TODO @seralpa create comm_args directly?
			int      num_borders = epsilod_num_borders(dims);
			bool     border_in_active[num_borders];
			bool     border_out_active[num_borders];
			int      index_comm_border[num_borders];
			HitRanks shifts_in[num_borders];
			HitRanks shifts_out[num_borders];

			/* Border status */
			EpsilodCommArgs comm_args;
			comm_args.border_in_active  = border_in_active;
			comm_args.border_out_active = border_out_active;
			comm_args.index_comm_border = index_comm_border;
			comm_args.shifts_in         = shifts_in;
			comm_args.shifts_out        = shifts_out;
			init_comm_args(&comm_args, stencil, lay);

			EpsilodTiles *p_tiles      = create_tiles(comm, lay, &globalMat, borders, comm_args);
			EpsilodTiles *p_tiles_copy = create_tiles(comm, lay, &globalMat, borders, comm_args);

			// Display extra tile info for debug
			if (epsilod_debug_tiles())
				dump_tiles(p_tiles);

			/* 4.6. Build distributed-memory communication pattern */
			CommCompIndex sorted_comm_indexes[num_borders];
			sort_comm_indexes(*p_tiles, sorted_comm_indexes);
			p_tiles->neighSync      = create_comm_pattern(comm, p_tiles, comm_args, sorted_comm_indexes, lay, HIT_CELL);
			p_tiles_copy->neighSync = create_comm_pattern(comm, p_tiles_copy, comm_args, sorted_comm_indexes, lay, HIT_CELL);

			// Kernel characterizations and thread spaces
			EpsilodThreads chars   = get_chars(dims, comm->type);
			EpsilodThreads threads = get_threads(*p_tiles);

			/* 4.8. Initialize array */
			print_once("Init stage...\n");
			fflush(stdout);

			EpsilodGlobalCoords coords = get_global_coords(*p_tiles, borders);

			// External/extra parameters
			Epsilod_ext *ext_params = (ext_params_arg == NULL) ? &(Epsilod_ext){0} : ext_params_arg;

			/* 4.8.1. First stage (Optional): initialization in host */
			if (f_init != NULL) {
				print_once("\tInitializing in the host...\n");
				fflush(stdout);
				f_init(p_tiles->mat, coords.mat, ext_params);
				Ctrl_HostTask(epsilod_host_touch, p_tiles->mat);
				Ctrl_MoveTo(comm, p_tiles->mat);
				Ctrl_WaitTile(comm, p_tiles->mat);
			}

			/* 4.8.2. Second stage (Optional): initialization in device */
			if (f_dev_init != NULL) {
				print_once("\tInitializing in the device...\n");
				fflush(stdout);
				f_dev_init(comm, threads.mat, chars.mat, 0, p_tiles->mat, coords.mat, ext_params);
			}

			/* 4.8.3. Initialize copy */
			if (f_init_copy == NULL) {
				#ifdef EPSILOD_INITIALIZE_COPY_IN_HOST
				print_once("\tInitializing copy in the host...\n");
				fflush(stdout);
				// Host: Initialize data in the copy
				char *omp_env     = getenv("OMP_NUM_THREADS");
				int   omp_threads = (omp_env != NULL) ? atoi(omp_env) : 1;
				#pragma omp parallel for num_threads(omp_threads)
				for (int i = 0; i < (p_tiles->mat).acumCard; i++)
					hit(p_tiles_copy->mat, i) = hit(p_tiles->mat, i);
				// Send tileCopy to the device
				Ctrl_MoveTo(comm, p_tiles_copy->mat);
				Ctrl_WaitTile(comm, p_tiles_copy->mat);
				#else // !EPSILOD_INITIALIZE_COPY_IN_HOST
				print_once("\tInitializing copy in the device...\n");
				fflush(stdout);
				Ctrl_Launch(comm, epsilod_dev_touch, threads.touch, chars.touch, p_tiles->mat);
				// This is limited by Controllers kernel thread id type, not by Ctrl_Thread
				if (p_tiles->mat.acumCard <= INT_MAX) {
					Ctrl_Launch(comm, epsilod_dev_copy_1d, threads.flat, chars.flat, p_tiles->mat, p_tiles_copy->mat);
				} else {
					switch (dims) {
						case 1: Ctrl_Launch(comm, epsilod_dev_copy_1d, threads.mat, chars.mat, p_tiles->mat, p_tiles_copy->mat); break;
						case 2: Ctrl_Launch(comm, epsilod_dev_copy_2d, threads.mat, chars.mat, p_tiles->mat, p_tiles_copy->mat); break;
						case 3: Ctrl_Launch(comm, epsilod_dev_copy_3d, threads.mat, chars.mat, p_tiles->mat, p_tiles_copy->mat); break;
						default:
							fprintf(stderr, "\nError: Matrix copy not implemented for more than 3 dimensions when total cardinality is greater than UINT_MAX.\n\n");
							MPI_Abort(MPI_COMM_WORLD, MPI_ERR_OTHER);
							exit(EXIT_FAILURE);
							break;
					}
				}
				#endif // EPSILOD_INITIALIZE_COPY_IN_HOST
				markTiles(comm, threads.touch, chars.touch, p_tiles, p_tiles_copy, &comm_args);
			} else {
				// NOTE: this exists like this for wavesim example. We should find a cleaner way to support it
				markTiles(comm, threads.touch, chars.touch, p_tiles, p_tiles_copy, &comm_args);
				swap(p_tiles, p_tiles_copy, EpsilodTiles *);
				compute(comm, f_init_copy, *p_tiles, *p_tiles_copy, threads, chars, coords, stencil, factor, ext_params);
				do_comms(comm, p_tiles, &comm_args);
				swap(p_tiles, p_tiles_copy, EpsilodTiles *);
			}

			#ifdef _EPS_ALB_EXP_MODE_
			expALB_init(numIterations);
			#endif //_EPS_ALB_EXP_MODE_

			hit_clockStop(init_clock);

			Ctrl_Synchronize();
			hit_comBarrier(lay);

			/* 4.9. Computation loop */
			print_once("Computation stage...\n");
			fflush(stdout);
			hit_clockStart(loop_clock);

			for (int iter = 0; iter < numIterations - 1; iter++) {
				hit_clockStart(iter_clock);

				swap(p_tiles, p_tiles_copy, EpsilodTiles *);
				compute(comm, f_updateCell, *p_tiles, *p_tiles_copy, threads, chars, coords, stencil, factor, ext_params);
				do_comms(comm, p_tiles, &comm_args);

				double k_time = Ctrl_TimeLastOp(comm, p_tiles->inner);
				hit_clockStart(redistribute_clock);
				bool is_ALB = EPSILOD_ALB(comm, &p_tiles, &p_tiles_copy, &coords, comm_args, &lay, &threads, stencil, HIT_CELL, k_time, (iter == (numIterations - 2)));
				// TODO move this inside EPSILOD_ALB to avoid checking if ALB was performed outside (requires kernel access from ALB)
				if (is_ALB) {
					Ctrl_Launch(comm, epsilod_dev_copy_1d, threads.flat, chars.flat, p_tiles->mat, p_tiles_copy->mat);
					Ctrl_WaitTile(comm, p_tiles->mat);
					// Only needed to prevent warnings
					markTiles(comm, threads.touch, chars.touch, p_tiles, p_tiles_copy, &comm_args);
				}
				hit_clockStop(redistribute_clock);
				hit_clockStop(iter_clock);
				#ifdef _EPS_ALB_EXP_MODE_
				expALB_print("&0& %d,%d,%lf,%lf,%lf,%d\n", hit_Rank, iter, iter_clock.seconds, redistribute_clock.seconds, k_time, is_ALB);
				#endif //_EPS_ALB_EXP_MODE_
			}

			/* 4.10. Last iteration update: no communication after */
			if (numIterations > 0) {
				swap(p_tiles, p_tiles_copy, EpsilodTiles *);
				compute(comm, f_updateCell, *p_tiles, *p_tiles_copy, threads, chars, coords, stencil, factor, ext_params);

				#ifdef _EPS_ALB_EXP_MODE_
				double k_time = Ctrl_TimeLastOp(comm, p_tiles->inner);
				expALB_print("&0& %d,%d,%lf,%lf,%lf,%d\n", hit_Rank, numIterations - 1, iter_clock.seconds, redistribute_clock.seconds, k_time, 0);
				#else //!_EPS_ALB_EXP_MODE_
				Ctrl_WaitTile(comm, p_tiles->inner);
				#endif //_EPS_ALB_EXP_MODE_
			}

			Ctrl_Synchronize();
			hit_clockStop(loop_clock);

			// Move matrix to the host:
			Ctrl_MoveFrom(comm, p_tiles->mat);
			Ctrl_WaitTile(comm, p_tiles->mat);

			print_once("Computation ended\n");
			fflush(stdout);

			/* 4.11. Clock results */
			hit_clockStop(ctrl_clock);
			hit_clockStop(comp_clock);
			#ifdef _EPS_ALB_EXP_MODE_
			expALB_print("&3& %lf\n", comp_clock.seconds);
			#endif //_EPS_ALB_EXP_MODE_

			reduceClocks(lay);
			print_clock_info();

			#ifdef _EPS_ALB_EXP_MODE_
			expALB_dump();
			#endif //_EPS_ALB_EXP_MODE_

			/* 4.12. Write result matrix */
			f_output(p_tiles->io, ext_params);

			print_once("Output finished\n");
			fflush(stdout);

			// Free tiles
			print_once("Freeing tiles...\n");
			fflush(stdout);
			freeEpsilodTiles(p_tiles);
			freeEpsilodTiles(p_tiles_copy);
		} else {
			/* 5. Inactive processes: only collective clock operations */
			fprintf(stderr, "[%d] Warning, process not active\n", hit_Rank);

			hit_clockStop(init_clock);
			hit_clockStop(ctrl_clock);
			hit_clockStop(loop_clock);

			reduceClocks(lay);
			print_clock_info();
		}

		/* 6. Free other resources */
		print_once("Freeing data structures...\n");
		fflush(stdout);
		Ctrl_Free(comm, stencil);
		hit_layFree(lay);

		print_once("Stopping distributed Controllers...\n");
		fflush(stdout);
		Ctrl_EndBlock();

		print_once("Epsilod End.\n");
		fflush(stdout);
	}
	#if CTRL_COUNTPARAM(EPSILOD_BASE_TYPE_COMPOUND) == 2
	hit_comFreeType(HIT_CELL);
	#endif // !CTRL_COUNTPARAM(EPSILOD_BASE_TYPE) == 2
}
