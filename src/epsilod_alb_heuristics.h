#ifndef _EPSILOD_HEURISTICS_ALB_
#define _EPSILOD_HEURISTICS_ALB_
/**
 * @file epsilod_alb_heuristics.h
 * @brief Prototypes for ALB Heuristics functions and structs for ALB Heuristics internal state.
 *
 * @copyright This software is part of the EPSILOD project by Trasgo Group, UVa.
 * The relevant license, warranty and copyright notice is available in the EPSILOD project repository.
 */

#include "epsilod_structs.h"
#include <unistd.h>

/**
 * Generic heuristic type
 */
typedef struct Heuristic {
	void *state;                                             /**< Internal state of the heuristic */
	void *(*init)();                                         /**< Init function, initializes state */
	bool (*check)(void *state, int curr_iter, int curr_ALb); /**< Check function, check if alb should be performed current iter */
	void (*redis)(void *state, int curr_iter, int curr_ALb,
				  HitTile_double all_times, HitTile_double avg_times, HitTile_double redis_times); /**< Redis function, called when a redistribution happens */
	void (*end)(void *state);                                                                      /**< End function, cleanup */
} Heuristic;

/**
 * Returns a newly created heuristic based on the environment variable \e EPSILOD_ALB_HEUR
 * Currently available options are:
 * 	\e NextALB strategy, in which we try to estimate in which iteration will a new ALB be needed
 * 	\e ConstIters strategy, in which we rebalance after a constant number of iterations
 * 	\e ExpIters strategy, in which we rebalance after a exponentially increasing number of iterations
 * 	\e DoubleIters strategy, in which we rebalance after an amount of iterations that doubles each time number of iterations
 * @note These options are case sensitive
 *
 * @return A heuristic object that follows the selected strategy
 */
Heuristic epsilod_get_heuristic();

#endif // _EPSILOD_HEURISTICS_ALB_