/**
 * @file gassimulation_ext_type.h
 * @brief Type declaration for external/extra parameters
 * 	Data type: float
 *
 * @copyright This software is part of the EPSILOD project by Trasgo Group, UVa.
 * The relevant license, warranty and copyright notice is available in the EPSILOD project repository.
 */

#include "gassimulation_types.h"

#define EPSILOD_USER_TYPES                   \
	typedef struct {                         \
		vec3f                   offsets[Q];  \
		unsigned char           opposite[Q]; \
		GASSIMULATION_CELL_TYPE wis[Q];      \
		GASSIMULATION_CELL_TYPE cellwidth;   \
		GASSIMULATION_CELL_TYPE deltaT;      \
		GASSIMULATION_CELL_TYPE tau;         \
	} Epsilod_ext;
