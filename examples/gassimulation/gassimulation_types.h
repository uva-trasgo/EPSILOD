/**
 * @file gassimulation_types.h
 * @brief Type definitions for gas simulation EPSILOD example.
 *
 * @copyright This software is part of the EPSILOD project by Trasgo Group, UVa.
 * The relevant license, warranty and copyright notice is available in the EPSILOD project repository.
 */
#ifndef _GASSIMULATION_TYPES_H_
#define _GASSIMULATION_TYPES_H_

#ifndef EPSILOD_FPGA_KERNELS
#include <math.h>

#include <stdlib.h>
#include <stdint.h>
#endif // !EPSILOD_FPGA_KERNELS

// #define GASSIMULATION_USE_FLOAT
#ifdef GASSIMULATION_USE_FLOAT
#define GASSIMULATION_CELL_TYPE float
#else
#define GASSIMULATION_CELL_TYPE double
#endif

// TODO Revisar if __CUDACC__
#define POW(a, b) powf(a, b)

#define VEC3_SCALE(v_out, v, scale) \
	{                               \
		v_out.x = v.x * scale;      \
		v_out.y = v.y * scale;      \
		v_out.z = v.z * scale;      \
	}

#define VEC3_ADD(v1, v2) \
	{                    \
		v1.x += v2.x;    \
		v1.y += v2.y;    \
		v1.z += v2.z;    \
	}

#define VEC3_DOT(v1, v2) (v1.x * v2.x + v1.y * v2.y + v1.z * v2.z)

#define VEC3_ADD_SCALED2(v_out, s1, v_in, s2) \
	{                                         \
		vec3f scaled;                         \
		VEC3_SCALE(scaled, v_in, s1)          \
		VEC3_SCALE(scaled, scaled, s2)        \
		VEC3_ADD(v_out, scaled)               \
	}

#define FLAG_OBSTACLE      -INFINITY
#define FLAG_KEEP_VELOCITY INFINITY

#define Q 19

typedef struct {
	GASSIMULATION_CELL_TYPE data[Q];
} cell_t;

#define EPSILOD_TYPE_COMPOUND_cell_t GASSIMULATION_CELL_TYPE, Q

typedef struct {
	GASSIMULATION_CELL_TYPE x, y, z;
} vec3f;

typedef struct {
	long x, y, z;
} vec3l;

#endif // _GASSIMULATION_TYPES_H_
