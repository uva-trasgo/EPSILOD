#ifndef _EPSILOD_ALB_
#define _EPSILOD_ALB_
/**
 * @file epsilod_alb.h
 * @brief Prototypes for ALB functions and structs for ALB data types.
 *
 * @copyright This software is part of the EPSILOD project by Trasgo Group, UVa.
 * The relevant license, warranty and copyright notice is available in the EPSILOD project repository.
 */

#include "hit_automatic_load_balancing/hit_alb.h"
#include "epsilod_structs.h"
#include "epsilod_alb_heuristics.h"

/**
 * @brief Rebalances the load of the computing nodes
 *
 * This is an adaptation from the work in hit_alb.c.
 * When it is time to rebalance the load, we first free the tileCopy, then we create a new
 * temporary tile and redistribute the old one to it. Then we free the old tile and create a
 * new tilecopy.
 *
 * The heuristics that are used to decide when to rebalance can be defined via function pointers. @see epsilod_alb_heuristics.h
 *
 * @param comm Controller object to allocate memory and interact with the device
 * @param pp_tiles Set of tiles. Overwriten when ALB is performed
 * @param pp_tiles_copy Auxiliary set of tiles. Overwriten when ALB is performed
 * @param p_coords Set of epsilod coordinates. Overwriten when ALB is performed
 * @param comm_args Communication arguments. Overwriten when ALB is performed
 * @param p_lay Pointer to the distributed layout. Overwriten when ALB is performed
 * @param p_threads Computation thread spaces for kernels. Overwriten when ALB is performed
 * @param stencil Weights for the stencl. Used to recalculate active borders
 * @param HIT_CELL Type for a stencil cell. Needed to compute the new communication patterns
 * @param time Time of the previous iteration inner kernel
 * @param is_last Whether or not is this the last iteration
 * @return Whether an alb was performed this iteration or not
 */
bool EPSILOD_ALB(PCtrl comm, EpsilodTiles **pp_tiles, EpsilodTiles **pp_tiles_copy, EpsilodGlobalCoords *p_coords, EpsilodCommArgs comm_args,
				 HitLayout *p_lay, EpsilodThreads *p_threads, HitTile_float stencil, HitType HIT_CELL, double time, bool is_last);

#endif // _EPSILOD_ALB_
