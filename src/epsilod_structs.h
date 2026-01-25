/**
 * @file epsilod_structs.h
 * @brief Epsilod: separate file for struct to avoid circular references
 *
 * @copyright This software is part of the EPSILOD project by Trasgo Group, UVa.
 * The relevant license, warranty and copyright notice is available in the EPSILOD project repository.
 */

#ifndef _EPSILOD_H_STRUCTS_
#define _EPSILOD_H_STRUCTS_

#define STR(a)  STR2(a)
#define STR2(a) #a

#include <math.h>

#include STR(EPSILOD_TYPES_INCLUDE)
#include "epsilod_types.h"
#include "Ctrl.h"

// HitTile types of base type, float for stencil weights, and double for alb
#if !EPSILOD_IS_FLOAT(EPSILOD_BASE_TYPE)
Ctrl_NewType(float);
#endif

#if !EPSILOD_IS_DOUBLE(EPSILOD_BASE_TYPE)
Ctrl_NewType(double);
#endif

// TODO @seralpa these probably should all be controlled from cmake (and documented somehow)
#ifndef _CTRL_EXAMPLES_EXP_MODE_
#define _EPSILOD_LAY_INFO_
#define _EPSILOD_TOPO_INFO_
#define _EPSILOD_WEIGHTS_INFO_
#endif // _CTRL_EXAMPLES_EXP_MODE_

#define EPSILOD_COMM_SORT
// #define EPSILOD_INITIALIZE_COPY_IN_HOST

Ctrl_NewType(EPSILOD_BASE_TYPE);

/* Special functions definition. */
typedef void (*stencilDeviceFunction)(PCtrl, Ctrl_Thread, Ctrl_Thread, int, HitTile(EPSILOD_BASE_TYPE), HitTile(EPSILOD_BASE_TYPE), EpsilodCoords, HitTile(float), float, Epsilod_ext *);
typedef void (*initDataDeviceFunction)(PCtrl, Ctrl_Thread, Ctrl_Thread, int, HitTile(EPSILOD_BASE_TYPE), EpsilodCoords, Epsilod_ext *);
typedef void (*initDataFunction)(HitTile(EPSILOD_BASE_TYPE), EpsilodCoords, Epsilod_ext *);
typedef void (*outputDataFunction)(HitTile(EPSILOD_BASE_TYPE), Epsilod_ext *);

/**
 * @brief Tiles needed in the EPSILOD computation process.
 */
typedef struct EpsilodTiles {
	HitTile(EPSILOD_BASE_TYPE) mat;                  /**< Encompasses the local region of the domain and the inbound halos */
	HitTile(EPSILOD_BASE_TYPE) inner;                /**< Selection of mat spanning the inner region of the local domain */
	HitTile(EPSILOD_BASE_TYPE) io;                   /**< Selection of mat used for input/output of the domain */
	HitTile(EPSILOD_BASE_TYPE) * border_in;          /**< Selections of mat spanning the inbound halos. Size 3^dims */
	HitTile(EPSILOD_BASE_TYPE) * border_out;         /**< Selections of mat spanning the outbound borders. Size 3^dims */
	HitTile(EPSILOD_BASE_TYPE) (*border_out_dev)[2]; /**< Selections of mat spanning the outbound borders in the device. Size 2*dims */
	HitPattern neighSync;                            /**< Communication pattern for this set of tiles */
} EpsilodTiles;

/**
 * @brief Device thread spaces used in computation
 */
typedef struct EpsilodThreads {
	Ctrl_Thread mat;                                 /**< Thread space for the device allocated tile */
	Ctrl_Thread inner;                               /**< Thread space for the inner tile selection */
	Ctrl_Thread flat;                                /**< Flattened thread space for the device allocated tile */
	Ctrl_Thread touch;                               /**< One-thread space used to avoid warnings */
	Ctrl_Thread border_out_dev[EPSILOD_MAX_DIMS][2]; /**< Thread spaces for the outbound tile selections in the device */
} EpsilodThreads;

/**
 * @brief Data that allows calculating global coordinates for each local subselection.
 */
typedef struct EpsilodGlobalCoords {
	EpsilodCoords mat;                                 /**< Global coordinates data for the device allocated tile */
	EpsilodCoords inner;                               /**< Global coordinates data for the inner tile selection */
	EpsilodCoords border_out_dev[EPSILOD_MAX_DIMS][2]; /**< Global coordinates data for the outbound tile selections in the device */
} EpsilodGlobalCoords;

/**
 * @brief Data needed in tile communications
 */
typedef struct EpsilodCommArgs {
	bool     *border_in_active;  /**< Whether inbound halos are active. Size 3^dims */
	bool     *border_out_active; /**< Whether outbound borders are active. Size 3^dims */
	HitRanks *shifts_in;         /**< HitRanks list, displacements to neighbors from which data is received.*/
	HitRanks *shifts_out;        /**< HitRanks list, displacements to neighbors to which data is sent.*/
	int      *index_comm_border; /**< An array of index ids for borders involved in communications. Size 3^dims */
} EpsilodCommArgs;

/**
 * @brief Enumeration of the different methods of domain partition
 */
typedef enum PartitionType {
	EPSILOD_PARTITION_MULTI_DIM,
	EPSILOD_PARTITION_SINGLE_DIM,
	EPSILOD_PARTITION_NOT_DIM,
	EPSILOD_PARTITION_WEIGHTED,
} PartitionType;

/**
 * Information used to create the partition
 *
 */
typedef struct PartitionInfo {
	PartitionType type; /**< Partition type */
	int           dims; /**< Number of active dimensions in the partition */
	int           dim;  /**< What dimension/s to partition or skip */
} PartitionInfo;

/**
 * ISL communication method
 */
typedef enum EpsilodCommMethod {
	HOST_WAITANY,
	HOST_WAITANY_RECVFIRST,
	HOST_WAITALL,
} EpsilodCommMethod;

/**
 * @brief Frees the space used by tile data
 * Frees the tiles, the lists of tiles in the structure (borders) and the structure itself
 * @param p_tiles Pointer to the tiles to free
 */
void freeEpsilodTiles(EpsilodTiles *p_tiles);

/**
 * @brief A helper structure to compare border tiles.
 * Stores the border index and the corresponding tile.
 */
typedef struct CommCompIndex {
	int index;                       /**< Border index associated with the tile */
	HitTile(EPSILOD_BASE_TYPE) tile; /**< The tile to be compared */
} CommCompIndex;

/**
 * Helper to check if a HitShape is not null.
 * @hideinitializer
 *
 * @param s Shape to check
 */
#define validShape(s) (hit_shapeDims((s)) != (-1))

/**
 * Swap 2 variables
 * @hideinitializer
 *
 * @param a Item to swap
 * @param b Item to swap
 * @param TYPE Type of \p a and \p b
 */
#define swap(a, b, TYPE)  \
	do {                  \
		TYPE SWAP = a;    \
		a         = b;    \
		b         = SWAP; \
	} while (0)

/* Color text modifiers shortcut */
#define BOLD_TEXT    "\e[1m"
#define REGULAR_TEXT "\e[m"

/**
 * Print by all processes. Disabled if Epsilod's experimentation mode is active.
 * @param format pointer to a null-terminated byte string specifying how to interpret the data
 * @param ... arguments specifying data to print.
 */
void print_all(const char *format, ...);

/**
 * Print by rank 0. Disabled if Epsilod's experimentation mode is active.
 * @param format pointer to a null-terminated byte string specifying how to interpret the data
 * @param ... arguments specifying data to print.
 */
void print_once(const char *format, ...);

/**
 * @brief Gather the given string buffer by rank 0 and print the gathered strings
 * @param buffer String buffer.
 * @param buffer_size Buffer size.
 * @param separator A string separator between the gathered buffers.
 */
void print_gather(char *buffer, size_t buffer_size, const char *separator);

/**
 * @brief Sorts border tiles indexes used in communication patterns
 * @param tiles Tiles used in EPSILOD.
 * @param[out] p_sorted_comm_indexes A pointer that will contain the sorted indexes along their corresponding border tiles.
 * The contents of the pointer are overwritten.
 */
void sort_comm_indexes(EpsilodTiles tiles, CommCompIndex *p_sorted_comm_indexes);

/**
 * @brief Initialize information used in communications,
 * such as borders status and their neighbours displacements.
 * @param p_comm_args Communications related data.
 * @param stencil The stencil tile.
 * @param lay The processor layout.
 */
void init_comm_args(EpsilodCommArgs *p_comm_args, HitTile_float stencil, HitLayout lay);

/**
 * @brief Create tile structures that EPSILOD needs to perform the stencil computation on the current process.
 * @param comm Pointer to the EPSILOD Controller.
 * @param lay The HitLayout used in the stencil computation.
 * @param global_mat A tile that represents the global domain.
 * @param borders Border sizes.
 * @param comm_args Communications related data.
 * @return An structure containing the generated tiles.
 */
EpsilodTiles *create_tiles(PCtrl comm, HitLayout lay, HitTile(EPSILOD_BASE_TYPE) * global_mat, EpsilodBorders borders, EpsilodCommArgs comm_args);

/**
 * @brief Create the comunication pattern needed to perform the stencil computation.
 * @param comm Pointer to the EPSILOD Controller.
 * @param p_tiles EPSILOD tiles structure.
 * @param comm_args Data needed for communications.
 * @param sorted_comm_indexes Sorted inbound border indexes.
 * @param lay The HitLayout used in the stencil computation.
 * @param HIT_CELL Hitmap type of domain cells.
 * @return
 */
HitPattern create_comm_pattern(PCtrl comm, EpsilodTiles *p_tiles, EpsilodCommArgs comm_args, CommCompIndex *sorted_comm_indexes, HitLayout lay, HitType HIT_CELL);

/**
 * @brief Whether EPSILOD should use device-aware MPI for communications.
 * @return true if device-aware MPI should be used, false otherwise.
 */
bool mpi_dev_aware();

/**
 * @brief Whether EPSILOD should run in experimentation mode.
 * This affects the verbosity of output and its format.
 *
 * @return true if experimentation mode is active, false otherwise.
 */
bool epsilod_exp_mode();

/**
 * @brief Whether EPSILOD should print tile debug information.
 *
 * @return true if tile debug information is expected, false otherwise.
 */
bool epsilod_debug_tiles();

/**
 * @brief Generates data that allows calculating global coordinates for each local subselection.
 * @param tiles EPSILOD tiles structure containing the local subselections.
 * @param borders Border sizes.
 * @return Global coordinates data for each subselection.
 */
EpsilodGlobalCoords get_global_coords(EpsilodTiles tiles, EpsilodBorders borders);

/**
 * @brief Gets the number of borders as the cube of the number of dimensions.
 * This counts the inner zone of the local domain as a "border".
 * @param dims The number of dimensions of the domain.
 * @return The number of borders.
 */
static inline int epsilod_num_borders(int dims) {
	return (int)pow(3, dims);
}

/**
 * @brief Sets threads used in computation tasks performed by EPSILOD.
 * @param tiles EPSILOD tiles structure containing the local subselections.
 * @return EPSILOD compute threads.
 */
EpsilodThreads get_threads(EpsilodTiles tiles);

/**
 * @brief Sets block sizes used in computation tasks performed by EPSILOD.
 * @param dims The number of domain dimensions.
 * @param ctrl_type EPSILOD controller type.
 * @return EPSILOD compute block sizes.
 */
EpsilodThreads get_chars(int dims, Ctrl_Type ctrl_type);

// TODO @davdiez move to hitmap and use in dumpShape
/**
 * Get the string representation of a shape in the given buffer
 *
 * @param[out] buff The output buffer.
 * @param sh \e HitShape A HitShape.
 *
 * @return
 */
int shape_to_str(char *buff, HitShape sh);

/**
 * Get the string representation of a tile in the given buffer
 *
 * @param[out] buffer The output buffer.
 * @param p_tile A pointer to the tile.
 */
int tile_to_str(char *buffer, HitTile(EPSILOD_BASE_TYPE) * p_tile);

/**
 * @brief Prints information about local subselection tiles.
 * @param p_tiles Local subselection tiles.
 */
void dump_tiles(EpsilodTiles *p_tiles);

/**
 * @brief Prints information about the global coordinates of local subselection tiles.
 * @param g_coords Global coordinates data of local tiles.
 */
void dump_global_coords(EpsilodGlobalCoords g_coords);

/**
 * @brief Gets data necesary to perform the selected partition scheme on the global tile.
 * @param dims The number of dimensions of the domain.
 * @return Partition data.
 */
PartitionInfo get_partition_info(int dims);

#ifdef _EPS_ALB_EXP_MODE_
/**
 * @brief Allocate memory for ALB exp output (currently 128 bytes per iter)
 * @param num_iters number of stencil iterations
 */
void expALB_init(int num_iters);

/**
 * @brief Add to the ALB exp output
 * @param format format string
 * @param ... variable srguments
 */
void expALB_print(const char *format, ...);

/**
 * @brief Print to stdout the ALB exp output and free buffer
 */
void expALB_dump();
#endif //_EPS_ALB_EXP_MODE_

#endif
