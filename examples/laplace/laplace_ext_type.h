/**
 * @file laplace_ext_type.h
 * @brief Type declaration for external/extra parameters
 * 	Data type: float
 *
 * @copyright This software is part of the EPSILOD project by Trasgo Group, UVa.
 * The relevant license, warranty and copyright notice is available in the EPSILOD project repository.
 */
#ifndef _EPSILOD_EXT_TYPES_H_
#define _EPSILOD_EXT_TYPES_H_

#include "laplace_types.h"

/* Default empty user type for extra parameters */
#ifndef EPSILOD_USER_TYPES
#define EPSILOD_USER_TYPES    \
	typedef struct {          \
		EPSILOD_BASE_TYPE dx; \
		EPSILOD_BASE_TYPE dy; \
	} Epsilod_ext;
#endif

#endif // EPSILOD_EXT_TYPES_H
