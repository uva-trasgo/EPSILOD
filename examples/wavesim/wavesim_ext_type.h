/**
 * @file wavesim_ext_type.h
 * @brief Type declaration for external/extra parameters
 *
 * @copyright This software is part of the EPSILOD project by Trasgo Group, UVa.
 * The relevant license, warranty and copyright notice is available in the EPSILOD project repository.
 */
#ifndef _EPSILOD_EXT_TYPES_H_
#define _EPSILOD_EXT_TYPES_H_

#include "wavesim_types.h"

/* User type for extra parameters */
#ifndef EPSILOD_USER_TYPES
#define EPSILOD_USER_TYPES           \
	typedef struct {                 \
		EPSILOD_BASE_TYPE dt;        \
		EPSILOD_BASE_TYPE dx;        \
		EPSILOD_BASE_TYPE dy;        \
		vec2f             center;    \
		EPSILOD_BASE_TYPE amplitude; \
		vec2f             sigma;     \
	} Epsilod_ext;
#endif

#endif // EPSILOD_EXT_TYPES_H
