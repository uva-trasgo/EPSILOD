/**
 * @file epsilod_alb_Heuristics.c
 * @brief ALB Heuristics functions.
 *
 * @copyright This software is part of the EPSILOD project by Trasgo Group, UVa.
 * The relevant license, warranty and copyright notice is available in the EPSILOD project repository.
 */

#include "epsilod_alb_heuristics.h"
#include <math.h>

/**
 * Internal state of NextALB heuristic
 * @see heur_nextALB
 */
typedef struct Heur_NextALB_State {
	int    nextALB;        /**< Iteration of next ALB */
	double avg_redis_time; /**< Average redistribution time*/
} Heur_NextALB_State;

/**
 * Internal state of the ExpIters heuristic
 * @see heur_expIters
 */
typedef struct Heur_ExpIters_State {
	int nextALB; /**< Iteration of next ALB */
} Heur_ExpIters_State;

/**
 * Internal state of the DoubleIters heuristic
 * @see heur_doubleIters
 */
typedef struct Heur_DoubleIters_State {
	int nextALB; /**< Iteration of next ALB */
} Heur_DoubleIters_State;

/**
 * Init function of the NextALB NextALB heuristic
 * @see heur_nextALB
 *
 * @return Heur_NextALB_State* internal state for nextALB heuristic
 */
void *Heur_NextALB_Init() {
	Heur_NextALB_State *state = (Heur_NextALB_State *)malloc(sizeof(Heur_NextALB_State));
	state->nextALB            = 0;
	state->avg_redis_time     = 0;
	return state;
}

/**
 * Check function of the NextALB heuristic
 * @see heur_nextALB
 *
 * @param state internal state of the heuristic
 * @param curr_iter current stencil iteration
 * @param curr_ALB current ALB iteration
 * @return whether ALB is needed this iteration
 */
bool Heur_NextALB_Check(void *state, int curr_iter, int curr_ALB) {
	Heur_NextALB_State *state_inner = (Heur_NextALB_State *)state;
	return curr_iter >= state_inner->nextALB;
}

/**
 * Redis function of the NextALB heuristic
 * @see heur_nextALB
 *
 * @param state internal state of the heuristic
 * @param curr_iter current stencil iteration
 * @param curr_ALB current ALB iteration
 * @param row_times average time per row for each process
 * @param avg_times average inner kernel time for each process
 * @param redis_times redistribution time for each process
 */
void Heur_NextALB_Redis(void *state, int curr_iter, int curr_ALB, HitTile_double row_times, HitTile_double avg_times, HitTile_double redis_times) {
	Heur_NextALB_State *state_inner = (Heur_NextALB_State *)state;
	double              sum_avg     = 0;
	double              worst       = 0;
	double              worst_redis = 0;

	for (int k = 0; k < hit_tileCard(avg_times); k++) {
		sum_avg += hit(avg_times, k);
		if (hit(avg_times, k) > worst) {
			worst = hit(avg_times, k);
		}
	}
	double avg = sum_avg / hit_tileCard(avg_times);

	int iters = 0;
	// redis times will be full of -1 on the first alb
	if (hit(redis_times, 0) != -1 && (worst - avg) != 0.0) {
		for (int k = 0; k < hit_tileCard(redis_times); k++) {
			if (hit(redis_times, k) > worst_redis) {
				worst_redis = hit(redis_times, k);
			}
		}
		state_inner->avg_redis_time = (((state_inner->avg_redis_time) * (curr_ALB - 1)) + worst_redis) / (curr_ALB);
		iters                       = (state_inner->avg_redis_time) / (worst - avg);
	}

	state_inner->nextALB = curr_iter + iters;
	#ifdef _EPS_ALB_EXP_MODE_
	expALB_print("&2& \"nextalb\",%d,%d,%d,%lf,%lf,%lf,%d,%d\n", hit_Rank, curr_iter, curr_ALB, state_inner->avg_redis_time, worst, avg, iters, state_inner->nextALB);
	#endif //_EPS_ALB_EXP_MODE_
}

/**
 * Init function of the ConstIters heuristic
 * @see heur_constIters
 *
 * @return int* internal state for constIters heuristic
 */
void *Heur_ConstIters_Init() {
	return malloc(sizeof(int));
}

/**
 * Check function of the ConstIters heuristic
 * @see heur_constIters
 *
 * @param state internal state of the heuristic
 * @param curr_iter current stencil iteration
 * @param curr_ALB current ALB iteration
 * @return
 */
bool Heur_ConstIters_Check(void *state, int curr_iter, int curr_ALB) {
	return true;
}

/**
 * Redis function of the ConstIters heuristic
 * @see heur_constIters
 *
 * @param state
 * @param curr_iter current stencil iteration
 * @param curr_ALB current ALB iteration
 * @param row_times average time per row for each process
 * @param avg_times average inner kernel time for each process
 * @param redis_times redistribution time for each process
 */
void Heur_ConstIters_Redis(void *state, int curr_iter, int curr_ALB, HitTile_double row_times, HitTile_double avg_times, HitTile_double redis_times) {
	#ifdef _EPS_ALB_EXP_MODE_
	double sum_avg = 0;
	double worst   = 0;

	for (int k = 0; k < hit_tileCard(avg_times); k++) {
		sum_avg += hit(avg_times, k);
		if (hit(avg_times, k) > worst) {
			worst = hit(avg_times, k);
		}
	}
	double avg = sum_avg / hit_tileCard(avg_times);
	expALB_print("&2& \"constiters\",%d,%d,%d,%lf,%lf\n", hit_Rank, curr_iter, curr_ALB, worst, avg);
	#endif //_EPS_ALB_EXP_MODE_
}

/**
 * Init function of the ExpIters heuristic
 * @see heur_expIters
 *
 * @return Heur_ExpIters_State* internal state for expIters heuristic
 */
void *Heur_ExpIters_Init() {
	Heur_ExpIters_State *state = (Heur_ExpIters_State *)malloc(sizeof(Heur_ExpIters_State));
	state->nextALB             = 0;
	return state;
}

/**
 * Check function of the ExpIters heuristic
 * @see heur_expIters
 *
 * @param state
 * @param curr_iter current stencil iteration
 * @param curr_ALB current ALB iteration
 * @return whether ALB is needed this iteration
 */
bool Heur_ExpIters_Check(void *state, int curr_iter, int curr_ALB) {
	Heur_ExpIters_State *state_inner = (Heur_ExpIters_State *)state;
	return curr_iter >= state_inner->nextALB;
}

/**
 * Redis function of the ExpIters heuristic
 * @see heur_expIters
 *
 * @param state
 * @param curr_iter current stencil iteration
 * @param curr_ALB current ALB iteration
 * @param row_times average time per row for each process
 * @param avg_times average inner kernel time for each process
 * @param redis_times redistribution time for each process
 */
void Heur_ExpIters_Redis(void *state, int curr_iter, int curr_ALB, HitTile_double row_times, HitTile_double avg_times, HitTile_double redis_times) {
	Heur_ExpIters_State *state_inner = (Heur_ExpIters_State *)state;
	state_inner->nextALB             = curr_iter + (int)pow(2, curr_ALB);

	#ifdef _EPS_ALB_EXP_MODE_
	double sum_avg = 0;
	double worst   = 0;

	for (int k = 0; k < hit_tileCard(avg_times); k++) {
		sum_avg += hit(avg_times, k);
		if (hit(avg_times, k) > worst) {
			worst = hit(avg_times, k);
		}
	}
	double avg = sum_avg / hit_tileCard(avg_times);
	expALB_print("&2& \"expiters\",%d,%d,%d,%lf,%lf,%d\n", hit_Rank, curr_iter, curr_ALB, worst, avg, state_inner->nextALB);
	#endif //_EPS_ALB_EXP_MODE_
}

/**
 * Init function of the DoubleIters heuristic
 * @see heur_doubleIters
 *
 * @return Heur_DoubleIters_State internal state for doubleIters heuristic
 */
void *Heur_DoubleIters_Init() {
	Heur_DoubleIters_State *state = (Heur_DoubleIters_State *)malloc(sizeof(Heur_DoubleIters_State));
	state->nextALB                = 0;
	return state;
}

/**
 * Check function of the DoubleIters heuristic
 * @see heur_doubleIters
 *
 * @param state internal state of the heuristic
 * @param curr_iter current stencil iteration
 * @param curr_ALB current ALB iteration
 * @return whether ALB is needed this iteration
 */
bool Heur_DoubleIters_Check(void *state, int curr_iter, int curr_ALB) {
	Heur_DoubleIters_State *state_inner = (Heur_DoubleIters_State *)state;
	return curr_iter >= (state_inner->nextALB);
}

/**
 * Redis function of the DoubleIters heuristic
 * @see heur_doubleIters
 *
 * @param state internal state of the heuristic
 * @param curr_iter current stencil iteration
 * @param curr_ALB current ALB iteration
 * @param row_times average time per row for each process
 * @param avg_times average inner kernel time for each process
 * @param redis_times redistribution time for each process
 */
void Heur_DoubleIters_Redis(void *state, int curr_iter, int curr_ALB, HitTile_double row_times, HitTile_double avg_times, HitTile_double redis_times) {
	Heur_DoubleIters_State *state_inner = (Heur_DoubleIters_State *)state;
	state_inner->nextALB                = curr_iter * 2;

	#ifdef _EPS_ALB_EXP_MODE_
	double sum_avg = 0;
	double worst   = 0;

	for (int k = 0; k < hit_tileCard(avg_times); k++) {
		sum_avg += hit(avg_times, k);
		if (hit(avg_times, k) > worst) {
			worst = hit(avg_times, k);
		}
	}
	double avg = sum_avg / hit_tileCard(avg_times);
	expALB_print("&2& \"doubleiters\",%d,%d,%d,%lf,%lf,%d\n", hit_Rank, curr_iter, curr_ALB, worst, avg, state_inner->nextALB);
	#endif //_EPS_ALB_EXP_MODE_
}

/**
 * Generic ending function for heuristics
 *
 * @param state internal state of the heuristic
 */
void Heur_End(void *state) {
	free(state);
}

Heuristic epsilod_get_heuristic() {
	const char *options[] = {"none", "NextALB", "ConstIters", "ExpIters", "DoubleIters", NULL};
	int         heur_idx  = hit_envOptions("EPSILOD_ALB_HEUR", options);

	// Check if partition is w, max dims is passed as it's only used for error checking irrelevant for this
	PartitionInfo part_info = get_partition_info(EPSILOD_MAX_DIMS);
	if (part_info.type != EPSILOD_PARTITION_WEIGHTED && heur_idx != 0) {
		print_once("Warning: ALB heuristic %s was selected but the partition is not weighted. Only weighted (w) partitions may use ALB. Disabling ALB");
		heur_idx = 0;
	}

	switch (heur_idx) {
		case 0: return (Heuristic){0};
		case 1: return (Heuristic){.state = NULL, .init = Heur_NextALB_Init, .check = Heur_NextALB_Check, .redis = Heur_NextALB_Redis, .end = Heur_End};
		case 2: return (Heuristic){.state = NULL, .init = Heur_ConstIters_Init, .check = Heur_ConstIters_Check, .redis = Heur_ConstIters_Redis, .end = Heur_End};
		case 3: return (Heuristic){.state = NULL, .init = Heur_ExpIters_Init, .check = Heur_ExpIters_Check, .redis = Heur_ExpIters_Redis, .end = Heur_End};
		case 4: return (Heuristic){.state = NULL, .init = Heur_DoubleIters_Init, .check = Heur_DoubleIters_Check, .redis = Heur_DoubleIters_Redis, .end = Heur_End};
		default:
			fprintf(stderr, "[epsilod_get_heuristic] Error: heuristic option out of bounds. heur_idx=%d", heur_idx);
			exit(EXIT_FAILURE);
	}
}
