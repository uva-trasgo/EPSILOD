#ifndef _EPSILOD_TYPES_H_
#define _EPSILOD_TYPES_H_
/**
 * @file epsilod_types.h
 * @brief Default type declarations for epsilod
 * 	- Default stencil base type: float
 * 	- Default external/extra parametres: int
 * 	- Other internal or interface types
 *
 * @copyright This software is part of the EPSILOD project by Trasgo Group, UVa.
 * The relevant license, warranty and copyright notice is available in the EPSILOD project repository.
 */

/* Default stencil base type */
#ifndef EPSILOD_BASE_TYPE
#define EPSILOD_BASE_TYPE float
#endif // EPSILOD_BASE_TYPE

/* Definitions to declare compound types */
#define EPSILOD_TYPE_COMPOUND(type)          EPSILOD_TYPE_COMPOUND_EXP(type)
#define EPSILOD_TYPE_COMPOUND_EXP(type)      EPSILOD_TYPE_COMPOUND_##type
#define EPSILOD_BASE_TYPE_COMPOUND           EPSILOD_TYPE_COMPOUND(EPSILOD_BASE_TYPE)
#define EPSILOD_GET_COMPOUND_TYPE(...)       EPSILOD_GET_COMPOUND_TYPE_EXP(__VA_ARGS__)
#define EPSILOD_GET_COMPOUND_TYPE_EXP(a, b)  a
#define EPSILOD_GET_COMPOUND_COUNT(...)      EPSILOD_GET_COMPOUND_COUNT_EXP(__VA_ARGS__)
#define EPSILOD_GET_COMPOUND_COUNT_EXP(a, b) b

/* Template like types for Tiles and KTiles */
#define HitTile(type)       HitTile_EXP(type)
#define HitTile_EXP(type)   HitTile_##type
#define HitTileR(type)      HitTileR_EXP(type)
#define HitTileR_EXP(type)  HitTileR_##type
#define KHitTile(type)      KHitTile_EXP(type)
#define KHitTile_EXP(type)  KHitTile_##type
#define KHitTileR(type)     KHitTileR_EXP(type)
#define KHitTileR_EXP(type) KHitTileR_##type

/* For conditional compilation based on the value of EPSILOD_BASE_TYPE, float or any other one */
#define EPSILOD_BASE_TYPE_float    1
#define EPSILOD_IS_FLOAT(type)     EPSILOD_IS_FLOAT_EXP(type)
#define EPSILOD_IS_FLOAT_EXP(type) EPSILOD_BASE_TYPE_##type

#define EPSILOD_BASE_TYPE_DOUBLE_double 1
#define EPSILOD_IS_DOUBLE(type)         EPSILOD_IS_DOUBLE_EXP(type)
#define EPSILOD_IS_DOUBLE_EXP(type)     EPSILOD_BASE_TYPE_DOUBLE_##type

/* Default extra parameters type */
#ifndef EPSILOD_USER_TYPES
#define EPSILOD_USER_TYPES \
	typedef struct {       \
		int foo;           \
	} Epsilod_ext;
#endif // !EPSILOD_USER_TYPES

/* Other types used in user declared functions */
#define EPSILOD_MAX_DIMS 4
#ifndef CTRL_USER_TYPES
#define CTRL_USER_TYPES                          \
	EPSILOD_USER_TYPES                           \
	typedef struct {                             \
		int low[EPSILOD_MAX_DIMS];               \
		int high[EPSILOD_MAX_DIMS];              \
	} EpsilodBorders;                            \
	typedef struct {                             \
		int            dims;                     \
		HitInd         size[EPSILOD_MAX_DIMS];   \
		HitInd         offset[EPSILOD_MAX_DIMS]; \
		EpsilodBorders borders;                  \
	} EpsilodCoords;
#endif // !CTRL_USER_TYPES

#endif // _EPSILOD_TYPES_H_
