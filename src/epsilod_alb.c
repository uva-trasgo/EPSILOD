/**
 * @file epsilod_alb.c
 * @brief ALB functions and structs for ALB data types.
 *
 * @copyright This software is part of the EPSILOD project by Trasgo Group, UVa.
 * The relevant license, warranty and copyright notice is available in the EPSILOD project repository.
 */

#include "epsilod_alb.h"

HitShape expandShapeBorders(HitTile *globalMat, HitInd *borderLow, HitInd *borderHigh, HitShape shape) {
	int dims = hit_shapeDims(shape);

	for (int i = 0; i < dims; i++) {
		/* DIM i FIRST BORDER IS NOT MINE */
		if (hit_sigIn(hit_shapeSig(shape, i), (hit_tileDimBegin(*globalMat, i) + borderLow[i]))) {
			shape = hit_shapeTransform(shape, i, HIT_SHAPE_BEGIN, -borderLow[i]);
		}
		/* DIM i LAST BORDER IS NOT MINE */
		if (hit_sigIn(hit_shapeSig(shape, i), (hit_tileDimEnd(*globalMat, i) - borderHigh[i]))) {
			shape = hit_shapeTransform(shape, i, HIT_SHAPE_END, +borderHigh[i]);
		}
	}
	return shape;
}

HitShape expandShapeBordersAndHalos(HitTile *globalMat, HitInd *borderLow, HitInd *borderHigh, HitShape shape) {
	int dims = hit_shapeDims(shape);

	for (int i = 0; i < dims; i++) {
		/* DIM i FIRST BORDER IS NOT MINE */
		shape = hit_shapeTransform(shape, i, HIT_SHAPE_BEGIN, -borderLow[i]);
		/* DIM i LAST BORDER IS NOT MINE */
		shape = hit_shapeTransform(shape, i, HIT_SHAPE_END, +borderHigh[i]);
	}
	return shape;
}

bool EPSILOD_ALB(PCtrl comm, EpsilodTiles **pp_tiles, EpsilodTiles **pp_tiles_copy, EpsilodGlobalCoords *p_coords, EpsilodCommArgs comm_args,
				 HitLayout *p_lay, EpsilodThreads *p_threads, HitTile_float stencil, HitType HIT_CELL, double time, bool is_last) {

	#ifdef DEBUG
	static HitClock call_clock = {HIT_CLOCK_STOPPED, -1, 0, 0, 0, 0};
	#endif // DEBUG
	static HitClock       redis_clock = {HIT_CLOCK_STOPPED, -1, 0, 0, 0, 0};
	static HitLayout      lay_comm; // Communications layout. All processes active
	static HitTile_double row_times;
	static HitTile_double avg_times;
	static HitTile_double redis_times;
	static HitAvg         avg;
	static MPI_Request    req_all_times;
	static MPI_Request    req_avg_times;
	static MPI_Request    req_redis_times;
	static int            curr_alb_iter = 0;
	static int            curr_iter     = 0;
	static bool           comm_times    = false;
	static Heuristic      heur;

	bool isALB = false;

	// First call to the function, initialization
	if (curr_iter == 0) {
		heur = epsilod_get_heuristic();
		if (heur.init != NULL) {
			avg        = hit_avgSimple(30);
			heur.state = heur.init();
			// only epsilod active processes participate
			lay_comm       = hit_layout_freeTopo(plug_layCopy, hit_topology(plug_topPlain), hit_shapeStd(1, 1));
			int comm_procs = lay_comm.topo.card[0];
			hit_tileDomainAlloc(&row_times, double, 1, comm_procs);
			hit_tileDomainAlloc(&avg_times, double, 1, comm_procs);
			hit_tileDomainAlloc(&redis_times, double, 1, comm_procs);
		}
	} else {
		#ifdef DEBUG
		hit_clockStop(call_clock);
		print_all("Process[%d] Time between calls: %lf\n", hit_Rank, call_clock.seconds);
		#endif // DEBUG
	}

	// NULL heur means never do ALB
	if (heur.check == NULL) {
		curr_iter++;
		return false;
	}

	// TODO @seralpa this should never happen because epsilod inactive procs don't enter this function
	if (!hit_layImActive(*p_lay)) time = 0.0;

	#ifdef DEBUG
	print_all("Process[%d] Time parameter: %lf\n", hit_Rank, time);
	#endif
	hit_avgInsertData(&avg, time);
	double average = hit_avgGetAvg(avg);

	if ((average != HITAVG_NOT_FULL) && (heur.check(heur.state, curr_iter, curr_alb_iter))) {
		if (!comm_times) { // First time that the data array is full and heur returns true comm the times across procs
			double zero = 0;
			hit_tileFill(&row_times, &zero);
			hit_tileFill(&avg_times, &zero);
			hit_tileFill(&redis_times, &zero);
			double timePerRow;
			if (!hit_layImActive(*p_lay))
				timePerRow = 0.0;
			else
				timePerRow = average / hit_tileDimCard((*pp_tiles)->mat, 0);

			double redisTime = redis_clock.seconds;

			// TODO @seralpa replace with hitmap api once it supports these kind of calls
			int ok = MPI_Iallgather(&timePerRow, 1, HIT_DOUBLE, row_times.data, 1, HIT_DOUBLE, lay_comm.pTopology[0]->comm, &req_all_times);
			hit_mpiTestError(ok, "Failed iallgather send");
			ok = MPI_Iallgather(&average, 1, HIT_DOUBLE, avg_times.data, 1, HIT_DOUBLE, lay_comm.pTopology[0]->comm, &req_avg_times);
			hit_mpiTestError(ok, "Failed iallgather send");
			ok = MPI_Iallgather(&redisTime, 1, HIT_DOUBLE, redis_times.data, 1, HIT_DOUBLE, lay_comm.pTopology[0]->comm, &req_redis_times);
			hit_mpiTestError(ok, "Failed iallgather send");

			comm_times = true;
		} else {
			hit_clockStart(redis_clock);
			int ok = MPI_Wait(&req_all_times, MPI_STATUS_IGNORE);
			hit_mpiTestError(ok, "Failed iallgather wait");
			ok = MPI_Wait(&req_avg_times, MPI_STATUS_IGNORE);
			hit_mpiTestError(ok, "Failed iallgather wait");
			ok = MPI_Wait(&req_redis_times, MPI_STATUS_IGNORE);
			hit_mpiTestError(ok, "Failed iallgather wait");

			curr_alb_iter++;
			isALB = true;

			heur.redis(heur.state, curr_iter, curr_alb_iter, row_times, avg_times, redis_times);

			double sum = 0;
			for (int k = 0; k < hit_tileCard(row_times); k++) {
				sum += hit(row_times, k);
			}

			// Compute new weights
			float normalizedWeights[hit_tileCard(row_times)];
			for (int k = 0; k < hit_tileCard(row_times); k++) {
				if (hit(row_times, k) == 0.0)
					normalizedWeights[k] = 0.0;
				else
					normalizedWeights[k] = (float)(sum / hit(row_times, k));
			}
			if (sum == 0.0) normalizedWeights[0] = 1;
			HitWeights weights = hitWeights(hit_tileCard(row_times), normalizedWeights);

			#ifdef DEBUG
			if (hit_layImLeader((lay_comm))) {
				printf("Process[%d] weights: ", hit_Rank);
				for (int i = 0; i < hit_tileCard(row_times); i++) {
					printf("%.5f ", normalizedWeights[i]);
				}
				printf("\n");
				fflush(stdout);
			}
			#endif

			EpsilodTiles  *p_tiles = *pp_tiles;
			EpsilodBorders borders = p_coords->inner.borders;

			// Move matrix to host
			Ctrl_MoveFrom(comm, p_tiles->mat);
			Ctrl_WaitTile(comm, p_tiles->mat);

			HitTile(EPSILOD_BASE_TYPE) *globalMat = (HitTile(EPSILOD_BASE_TYPE) *)hit_tileRoot(&p_tiles->mat);

			// Free old tilecopy
			freeEpsilodTiles(*pp_tiles_copy);

			// Create new layout
			HitLayout new_lay = hit_layout_freeTopo(plug_layDimWeighted_Blocks, hit_topology(plug_topPlain), p_lay->origShape, 0, weights);

			if (!epsilod_exp_mode()) {
				printf("[%d] new_lay->shape: ", hit_Rank);
				dumpShape(new_lay.shape);
				fflush(stdout);

				printf("[%d] new_lay->orig: ", hit_Rank);
				dumpShape(new_lay.origShape);
				fflush(stdout);
			}

			// Create new tiles
			int dims        = hit_layNumDims(new_lay);
			int num_borders = epsilod_num_borders(dims);

			init_comm_args(&comm_args, stencil, new_lay);

			// Compute new tiles
			EpsilodTiles *p_new_tiles = create_tiles(comm, new_lay, globalMat, borders, comm_args);

			// Initialize array
			print_once("ALB Redistribution\n");
			print_once("\nPartition weights = {");
			for (int i = 0; i < weights.num_procs; i++)
				print_once(" %f,", weights.ratios[i]);
			print_once("\b }\n");
			fflush(stdout);

			// Redistribute
			hit_patternDoOnce(hit_patternLayRedistributeGeneric(*p_lay, new_lay, &p_tiles->mat, &p_new_tiles->mat, HIT_FLOAT, expandShapeBorders, expandShapeBordersAndHalos));

			// Update layout
			hit_layFree(*p_lay);
			*p_lay = new_lay;

			// Free old tiles
			freeEpsilodTiles(p_tiles);

			// Compute new tiles copy
			EpsilodTiles *p_new_tiles_copy = create_tiles(comm, new_lay, globalMat, borders, comm_args);

			// Compute new comm patterns
			CommCompIndex sorted_comm_indexes[num_borders];
			sort_comm_indexes(*p_new_tiles, sorted_comm_indexes);
			p_new_tiles->neighSync      = create_comm_pattern(comm, p_new_tiles, comm_args, sorted_comm_indexes, new_lay, HIT_CELL);
			p_new_tiles_copy->neighSync = create_comm_pattern(comm, p_new_tiles_copy, comm_args, sorted_comm_indexes, new_lay, HIT_CELL);

			*p_threads = get_threads(*p_new_tiles);
			*p_coords  = get_global_coords(*p_new_tiles, borders);

			// Communicate halos
			hit_patternDo(p_new_tiles->neighSync);

			*pp_tiles      = p_new_tiles;
			*pp_tiles_copy = p_new_tiles_copy;

			// Move matrix to device
			// NOTE this causes a warning due to mat not being initialized on the host, a host task to mark it as initialized can't be launched from here
			Ctrl_MoveTo(comm, p_new_tiles->mat);
			Ctrl_WaitTile(comm, p_new_tiles->mat);

			// Reset average
			hit_avgResetData(&avg);
			comm_times = false;
			#ifdef _EPS_ALB_EXP_MODE_
			expALB_print("&1& %d,", hit_Rank);
			expALB_print(" %f", weights.ratios[0]);
			for (int i = 1; i < weights.num_procs; i++)
				expALB_print(", %f", weights.ratios[i]);
			expALB_print("\n");
			#endif //_EPS_ALB_EXP_MODE_
			hit_clockStop(redis_clock);
		}
	}
	curr_iter++;
	if (is_last) {
		heur.end(heur.state);
		hit_layFree(lay_comm);
		hit_tileFree(row_times);
		hit_tileFree(avg_times);
		hit_tileFree(redis_times);
	}
	#ifdef DEBUG
	hit_clockStart(call_clock);
	#endif // DEBUG
	return isALB;
}

#ifdef _EPS_ALB_EXP_MODE_
char *expALB_buf_start;
char *expALB_buf_head;
int   expALB_avail_space;

void expALB_init(int num_iters) {
	expALB_avail_space = num_iters * 128;
	expALB_buf_head    = malloc(expALB_avail_space);
	expALB_buf_start   = expALB_buf_head;
}

void expALB_print(const char *format, ...) {
	va_list args;
	va_start(args, format);
	int n_chars = vsnprintf(expALB_buf_head, expALB_avail_space, format, args);
	if (n_chars < 0) {
		fprintf(stderr, "[expALB_print] Error: vsprintf failed with code %d\n", n_chars);
		exit(EXIT_FAILURE);
	}
	expALB_avail_space -= n_chars;
	expALB_buf_head += n_chars;
	if (expALB_avail_space <= 0) {
		fprintf(stderr, "[expALB_print] Error: buffer run out of space\n");
		exit(EXIT_FAILURE);
	}
	va_end(args);
}

void expALB_dump() {
	printf("\n%s\n", expALB_buf_start);
	fflush(stdout);
	free(expALB_buf_start);
}
#endif //_EPS_ALB_EXP_MODE_
