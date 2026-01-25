/**
 * @file gaussian_ext_type.h
 * @brief Epsilod: Default type declaration for external/extra parameters
 *
 * @copyright This software is part of the EPSILOD project by Trasgo Group, UVa.
 * The relevant license, warranty and copyright notice is available in the EPSILOD project repository.
 */

#ifndef _EPSILOD_EXT_TYPES_H_
#define _EPSILOD_EXT_TYPES_H_

/* Default empty user type for extra parameters */
#ifndef EPSILOD_USER_TYPES
#define EPSILOD_USER_TYPES \
	typedef struct {       \
		int kw;            \
	} Epsilod_ext;
#endif

#endif // EPSILOD_EXT_TYPES_H
