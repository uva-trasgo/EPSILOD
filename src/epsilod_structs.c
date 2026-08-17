/**
 * @file epsilod_structs.c
 * @brief Epsilod: separate file for struct to avoid circular references
 *
 * @copyright This software is part of the EPSILOD project by Trasgo Group, UVa.
 * The relevant license, warranty and copyright notice is available in the EPSILOD project repository.
 */

#include "epsilod_structs.h"
#include "epsilod_env.h"
#include "epsilod_log.h"

HitTile(EPSILOD_BASE_TYPE) EPSILOD_TILE_NULL = HIT_TILE_NULL_STATIC;

/**
 * @brief Transform border coordinates in flat border number
 * @param ndims Number of dimensions
 * @param dim The chosen dimension
 * @param displ +1 or -1 to indicate the direction (low, high)
 * @return The flat number of the border
 */
int epsilod_border_number(int ndims, int dim, int displ) {
	int mid       = epsilod_num_borders(ndims) / 2;
	int acum_card = (int)pow(3, ndims - 1 - dim);

	return mid + displ * acum_card;
}

/**
 * @brief Compare tiles to order communications.
 * Compares by number of elements. This comparator function results in a bigger first sorting.
 * This doesn't take into account that memory transfers for tiles with non-consecutive elements in memory
 * may take longer even if the number of elements is smaller.
 *
 * @param a A tile to be compared, of the helper type CommCompIndex
 * @param b A tile to be compared, of the helper type CommCompIndex
 * @return -1 if the first tile is bigger, 1 if it is smaller, 0 if they are the same size.
 */
int compare_comm_tiles(const void *a, const void *b) {
	// qsort sorts in ascending order
	// a less than b -> return negative
	CommCompIndex index_a = *(CommCompIndex *)a;
	CommCompIndex index_b = *(CommCompIndex *)b;
	if (index_a.tile.acumCard > index_b.tile.acumCard)
		return -1;
	if (index_a.tile.acumCard < index_b.tile.acumCard)
		return 1;
	return 0;
}

/**
 * @brief Sets the number of kernel compute threads to spawn based on the cardinalities of a tile.
 * It will generate threads on the same dimensional space as the tile.
 *
 * @param p_tile A pointer to the tile of reference.
 * @return A Ctrl_Thread with the corresponding number of threads for each dimension.
 */
Ctrl_Thread init_thread_from_tile(HitTile(EPSILOD_BASE_TYPE) * p_tile) {
	if (hit_tileIsNull(*p_tile))
		return CTRL_THREAD_NULL;

	Ctrl_Thread tile_threads = CTRL_THREAD_NULL;
	switch (hit_tileDims(*p_tile)) {
		case 1:
			Ctrl_ThreadInit(tile_threads,
							hit_tileDimCard(*p_tile, 0));
			break;
		case 2:
			Ctrl_ThreadInit(tile_threads,
							hit_tileDimCard(*p_tile, 0),
							hit_tileDimCard(*p_tile, 1));
			break;
		case 3:
			Ctrl_ThreadInit(tile_threads,
							hit_tileDimCard(*p_tile, 0),
							hit_tileDimCard(*p_tile, 1),
							hit_tileDimCard(*p_tile, 2));
			break;
		case 4:
			Ctrl_ThreadInit(tile_threads,
							hit_tileDimCard(*p_tile, 1),
							hit_tileDimCard(*p_tile, 2),
							hit_tileDimCard(*p_tile, 3));
			break;
	}
	return tile_threads;
}

/**
 * @brief Generates shapes for device tiles corresponding to the outbound data of the local tile.
 * Generates the minimum number of non-overlapping shapes to reduce the number of device transfers without data replication.
 * Skips shapes based on inactive borders.
 *
 * @param lay The layout
 * @param borders Border sizes.
 * @param p_border_out_active Array indicating if outbound borders are active.
 * @param[out] shp_border_out_dev Shapes of outbound border tiles for the device.
 */
void build_outdev_shapes(HitLayout lay, EpsilodBorders borders, bool *p_border_out_active, HitShape (*shp_border_out_dev)[2]) {
	int dims = hit_layNumDims(lay);
	for (int j = 0; j < dims; j++) {
		// Number of borders
		// Invert the border number (is an out number)
		int border_low  = epsilod_border_number(dims, j, +1);
		int border_high = epsilod_border_number(dims, j, -1);

		// If neighbor does not exist, there is not a border
		if (!p_border_out_active[border_low]) {
			// NULL signature
			shp_border_out_dev[j][0] = HIT_SIG_SHAPE_NULL;
		} else {
			shp_border_out_dev[j][0] = hit_shapeTransform(hit_layShape(lay), j, HIT_SHAPE_FIRST, borders.high[j]);

			// Take out the parts which are overlapped with borders in previous dims
			for (int k = 0; k < j; k++) {
				if (validShape(shp_border_out_dev[k][0])) {
					shp_border_out_dev[j][0] = hit_shapeTransform(shp_border_out_dev[j][0], k, HIT_SHAPE_BEGIN, borders.high[k]);
				}
			}
		}

		if (!p_border_out_active[border_high]) {
			// NULL signature
			shp_border_out_dev[j][1] = HIT_SIG_SHAPE_NULL;
		} else {
			shp_border_out_dev[j][1] = hit_shapeTransform(hit_layShape(lay), j, HIT_SHAPE_LAST, borders.low[j]);

			// Take out the parts which are overlapped with borders in previous dims
			for (int k = 0; k < j; k++) {
				if (validShape(shp_border_out_dev[k][1])) {
					shp_border_out_dev[j][1] = hit_shapeTransform(shp_border_out_dev[j][1], k, HIT_SHAPE_END, -borders.low[k]);
				}
			}
		}
	}

	// Clear shape (NULL) if a range is empty due to the elimination of overlapped parts
	for (int j = 0; j < dims; j++) {
		for (int k = 0; k < dims; k++) {
			if (hit_shapeSig(shp_border_out_dev[j][0], k).begin > hit_shapeSig(shp_border_out_dev[j][0], k).end) {
				shp_border_out_dev[j][0] = HIT_SIG_SHAPE_NULL;
				break;
			}
		}
	}
	for (int j = 0; j < dims; j++) {
		for (int k = 0; k < dims; k++) {
			if (hit_shapeSig(shp_border_out_dev[j][1], k).begin > hit_shapeSig(shp_border_out_dev[j][1], k).end) {
				shp_border_out_dev[j][1] = HIT_SIG_SHAPE_NULL;
				break;
			}
		}
	}
}

/**
 * @brief Creates and allocates the local tile.
 * This tile contains the necessary data for the current process to perform computation. That is:
 * the local part of the domain partition, given by the layout, and an outer inbound halo to receive data from other processes.
 *
 * @param comm Pointer to the EPSILOD Controller.
 * @param global_mat The global tile spanning the whole domain.
 * @param shp_lay The layout shape.
 * @param borders Border sizes.
 * @return The local tile.
 */
HitTile(EPSILOD_BASE_TYPE) create_tile_mat(PCtrl comm, HitTile(EPSILOD_BASE_TYPE) * global_mat, HitShape shp_lay, EpsilodBorders borders) {
	HitShape shp_expanded = shp_lay;
	for (int i = 0; i < hit_shapeDims(shp_lay); i++) {
		shp_expanded = hit_shapeTransform(shp_expanded, i, HIT_SHAPE_BEGIN, -borders.low[i]);
		shp_expanded = hit_shapeTransform(shp_expanded, i, HIT_SHAPE_END, borders.high[i]);
	}
	HitTile(EPSILOD_BASE_TYPE) mat = Ctrl_Select(EPSILOD_BASE_TYPE, *global_mat, shp_expanded, CTRL_SELECT_ARR_COORD);
	if (epsilod_align() == EPSILOD_MEM_ALIGN_NONE)
		Ctrl_Alloc(comm, mat);
	else
		Ctrl_Alloc(comm, mat, CTRL_MEM_ALIGNED);
	return mat;
}

/**
 * @brief Creates a tile spanning the inner area of the local tile. The inner area excludes inbound halos and outbound borders.
 * @param mat The local tile.
 * @param lay The layout.
 * @param borders Border sizes.
 * @return A tile spanning the inner part of the local tile.
 */
HitTile(EPSILOD_BASE_TYPE) create_tile_inner(HitTile(EPSILOD_BASE_TYPE) * mat, HitLayout lay, bool *p_active_borders_out, EpsilodBorders borders) {

	HitShape shp_inner = lay.shape;

	for (int j = 0; j < hit_tileDims(*mat); j++) {
		int border_low  = epsilod_border_number(hit_layNumDims(lay), j, -1);
		int border_high = epsilod_border_number(hit_layNumDims(lay), j, +1);

		if (p_active_borders_out[border_low]) {
			// Trim off border high
			shp_inner = hit_shapeTransform(shp_inner, j, HIT_SHAPE_END, -borders.high[j]);
		}
		if (p_active_borders_out[border_high]) {
			// Trim off border low
			shp_inner = hit_shapeTransform(shp_inner, j, HIT_SHAPE_BEGIN, borders.low[j]);
		}
	}

	return Ctrl_Select(EPSILOD_BASE_TYPE, *mat, shp_inner, CTRL_SELECT_ARR_COORD);
}

/**
 * @brief Creates a tile used when computing the inner area of the local tile. The inner area excludes inbound halos and outbound borders.
 * The actual shape depends on Epsilod's memory alignment model of the local tile.
 * If the memory alignment mode is EPSILOD_MEM_ALIGN_THREADS, the generated tile's last dimension is extended to the beginning.
 * Otherwise, the tile is virtually the same as the inner tile.
 * @param mat The local tile.
 * @param inner The inner region tile.
 * @return A tile to compute the inner part of the local tile.
 */
HitTile(EPSILOD_BASE_TYPE) create_tile_inner_compute(HitTile(EPSILOD_BASE_TYPE) * mat, HitTile(EPSILOD_BASE_TYPE) * inner) {

	if (epsilod_align() != EPSILOD_MEM_ALIGN_THREADS || hit_tileDims(*mat) == 1)
		return *inner;

	HitShape shp_mat   = mat->shape;
	HitShape shp_inner = inner->shape;

	int    last_dim = hit_shapeDims(shp_inner) - 1;
	HitInd inner_last_dim_offset =
		hit_shapeSig(shp_inner, last_dim).begin -
		hit_shapeSig(shp_mat, last_dim).begin;

	shp_inner = hit_shapeTransform(shp_inner, last_dim, HIT_SHAPE_BEGIN, -inner_last_dim_offset);

	return Ctrl_Select(EPSILOD_BASE_TYPE, *mat, shp_inner, CTRL_SELECT_ARR_COORD);
}

/**
 * @brief Creates a tile to perform IO operations.
 * It spans the local part of the domain partition including global matrix borders but excluding inbound halos.
 *
 * @param mat The local tile.
 * @param global_mat The global tile spanning the whole domain.
 * @param borders
 * @return A tile spanning the local part of the domain partition.
 */
HitTile(EPSILOD_BASE_TYPE) create_tile_io(HitTile(EPSILOD_BASE_TYPE) * mat, HitTile(EPSILOD_BASE_TYPE) global_mat, EpsilodBorders borders) {
	// Eliminate borders except if they are global for io selection
	HitShape shp_io = mat->shape;
	for (int i = 0; i < hit_tileDims(global_mat); i++) {
		// Dim i first border is not mine
		if (!hit_sigIn(hit_shapeSig(shp_io, i), hit_tileDimBegin(global_mat, i))) {
			shp_io = hit_shapeTransform(shp_io, i, HIT_SHAPE_BEGIN, borders.low[i]);
		}
		// Dim i last border is not mine
		if (!hit_sigIn(hit_shapeSig(shp_io, i), hit_tileDimEnd(global_mat, i))) {
			shp_io = hit_shapeTransform(shp_io, i, HIT_SHAPE_END, -borders.high[i]);
		}
	}
	return Ctrl_Select(EPSILOD_BASE_TYPE, *mat, shp_io, CTRL_SELECT_ARR_COORD);
}

/**
 * @brief Creates a shape spanning an inbound halo.
 * This defines data in the local tile to be received from other processes.
 *
 * @param shp_lay The layout shape.
 * @param active Whether the halo is active.
 * @param borders Border sizes.
 * @param shift_in Displacement to halo's neighbor.
 * @return A shape spanning an inbound halo.
 */
HitShape create_shape_borderin(HitShape shp_lay, bool active, EpsilodBorders borders, HitRanks shift_in) {
	// Non-active borders, null shape
	if (!active)
		return HIT_SHAPE_NULL;

	HitShape shp_border_in = shp_lay;
	// Extract ranks for this border
	for (int j = 0; j < hit_shapeDims(shp_lay); j++) {
		if (shift_in.rank[j] == -1) {
			shp_border_in = hit_shapeTransform(shp_border_in, j, HIT_SHAPE_FIRST, borders.low[j]);
			shp_border_in = hit_shapeTransform(shp_border_in, j, HIT_SHAPE_MOVE, -borders.low[j]);
		} else if (shift_in.rank[j] == 1) {
			shp_border_in = hit_shapeTransform(shp_border_in, j, HIT_SHAPE_LAST, borders.high[j]);
			shp_border_in = hit_shapeTransform(shp_border_in, j, HIT_SHAPE_MOVE, borders.high[j]);
		}
	}
	return shp_border_in;
}

/**
 * @brief Creates a shape spanning an outbound border.
 * This defines data in the local tile to be sent to other processes.
 *
 * @param shp_lay The layout shape.
 * @param active Whether the border is active.
 * @param borders Border sizes.
 * @param shift_in Displacement to halo's neighbor.
 * @return A shape spanning an outbound border.
 */
HitShape create_shape_borderout(HitShape shp_lay, bool active, EpsilodBorders borders, HitRanks shift_in) {
	// Non-active borders, null shape
	if (!active)
		return HIT_SHAPE_NULL;

	HitShape shp_border_out = shp_lay;
	// Extract ranks for this border
	for (int j = 0; j < hit_shapeDims(shp_lay); j++) {
		if (shift_in.rank[j] == -1) {
			shp_border_out = hit_shapeTransform(shp_border_out, j, HIT_SHAPE_LAST, borders.low[j]);
		} else if (shift_in.rank[j] == 1) {
			shp_border_out = hit_shapeTransform(shp_border_out, j, HIT_SHAPE_FIRST, borders.high[j]);
		}
	}
	return shp_border_out;
}

/**
 * @brief Creates a tile spanning an inbound halo or an outbound border.
 *
 * @see See create_shape_borderin(), create_shape_borderout()
 *
 * @param p_mat The local tile.
 * @param shp_border The border shape.
 * @param active Whether the border is active.
 * @return A tile spanning a border.
 */
HitTile(EPSILOD_BASE_TYPE) create_tile_border(HitTile(EPSILOD_BASE_TYPE) * p_mat, HitShape shp_border, bool active) {
	// Non-active borders, null tiles
	if (!active)
		return EPSILOD_TILE_NULL;

	return Ctrl_Select(EPSILOD_BASE_TYPE, *p_mat, shp_border, CTRL_SELECT_ARR_COORD);
}

/**
 * @brief Creates tiles spanning outbound borders in the device.
 * This allows performing computation independently of the inner part.
 * @param[inout] p_tiles EPSILOD tiles structure to be filled with outbound device borders.
 * @param lay The layout.
 * @param p_border_out_active Array indicating if outbound borders are active.
 * @param borders Border sizes.
 */
void create_tile_borderoutdev(EpsilodTiles *p_tiles, HitLayout lay, bool *p_border_out_active, EpsilodBorders borders) {
	int      dims = hit_layNumDims(lay);
	HitShape shp_border_outdev[dims][2];
	build_outdev_shapes(lay, borders, p_border_out_active, shp_border_outdev);
	for (int i = 0; i < dims; i++) {
		for (int j = 0; j < 2; j++) {
			if (validShape(shp_border_outdev[i][j])) {
				p_tiles->border_out_dev[i][j] = Ctrl_Select(EPSILOD_BASE_TYPE, p_tiles->mat, shp_border_outdev[i][j], CTRL_SELECT_ARR_COORD);
			} else {
				p_tiles->border_out_dev[i][j] = EPSILOD_TILE_NULL;
			}
		}
	}
}

/**
 * Creates and allocates a new empty tile with the same shape as the original tile.
 * If the original is a null tile, it returns a null tile.
 * @param comm Pointer to the EPSILOD Controller.
 * @param tile The original tile.
 * @return The new copy tile or a null tile.
 */
HitTile(EPSILOD_BASE_TYPE) create_tile_copy(PCtrl comm, HitTile(EPSILOD_BASE_TYPE) tile) {
	if (hit_tileIsNull(tile))
		return EPSILOD_TILE_NULL;
	return Ctrl_DomainAlloc(comm, EPSILOD_BASE_TYPE, tile.shape);
}

/**
 * @brief Merge inbound border tiles to minimize the number of buffers while keeping buffer contiguity.
 * Two tiles should be adjacent to be expressed as selections of a larger buffer.
 * A tile can be expressed as a selection of another while keeping data contiguity if their last n-1 dimensions match.
 *
 * @param dims
 * @param shp_border
 * @param shp_border_expanded
 * @param merged_indexes
 */
void calc_borderin_merge(int dims, HitShape *shp_border, HitShape *shp_border_expanded, int *merged_indexes) {
	int  num_borders = epsilod_num_borders(dims);
	int *p_merge_dim = malloc(sizeof(int) * num_borders);
	for (int i = 0; i < num_borders; i++) {
		merged_indexes[i]      = i;
		shp_border_expanded[i] = shp_border[i];
		p_merge_dim[i]         = -1;
		if (hit_shapeCmp(shp_border[i], HIT_SHAPE_NULL))
			continue;
		// Brute force
		for (int j = 0; j < num_borders; j++) {
			if (i == j)
				continue;
			// Check if shp_border[i] is adjacent to shp_border[j]
			for (int adj_d = 0; adj_d < dims; adj_d++) {
				bool is_adj = true;
				for (int d = 0; d < dims; d++) {
					HitSig sig_a = hit_shapeSig(shp_border[i], d);
					HitSig sig_b = hit_shapeSig(shp_border[j], d);
					if (d == adj_d) {
						// Checking adjacency dim
						if (!(sig_a.begin - sig_a.stride == sig_b.end ||
							  sig_a.end + sig_a.stride == sig_b.begin)) {
							is_adj = false;
							break;
						}
					} else {
						// Checking the other dims
						if (sig_a.begin != sig_b.begin ||
							sig_a.end != sig_b.end) {
							is_adj = false;
							break;
						}
					}
				}
				if (is_adj) {
					// Only merge tiles to bigger tiles
					if (hit_shapeCard(shp_border[i]) >= hit_shapeCard(shp_border[j]))
						continue;
					bool keep_contiguity = true;
					// Check if last n-1 dim cards match
					for (int d = 1; d < dims; d++) {
						if (hit_shapeSigCard(shp_border[i], d) != hit_shapeSigCard(shp_border[j], d)) {
							keep_contiguity = false;
							break;
						}
					}
					if (keep_contiguity) {
						merged_indexes[i] = j;
						p_merge_dim[i]    = adj_d;

						// A tile may only be merged once
						break;
					}
				}
			}
		}
	}

	for (int i = 0; i < num_borders; i++) {
		int to_idx = merged_indexes[i];
		if (to_idx == i)
			continue;

		int transform_dim = p_merge_dim[i];
		// Expand the shape
		HitSig sig_a = hit_shapeSig(shp_border[i], transform_dim);
		HitSig sig_b = hit_shapeSig(shp_border[to_idx], transform_dim);
		int    action, dir;
		if (sig_a.end < sig_b.begin) {
			action = HIT_SHAPE_BEGIN;
			dir    = -1;
		} else {
			action = HIT_SHAPE_END;
			dir    = 1;
		}
		shp_border_expanded[to_idx] = hit_shapeTransform(shp_border_expanded[to_idx], transform_dim, action, dir * hit_shapeSigCard(shp_border[i], transform_dim));

		shp_border_expanded[i] = HIT_SHAPE_NULL;
	}
	free(p_merge_dim);
}

/**
 * @brief Merge outbound border tiles to minimize data duplication while keeping buffer contiguity.
 * A tile should be contained in another to be expressed as a selection.
 * A tile can be expressed as a selection of another while keeping data contiguity if their last n-1 dimensions match.
 *
 * @param dims
 * @param shp_border
 * @param merged_indexes
 */
void calc_borderout_merge(int dims, HitShape *shp_border, int *merged_indexes) {
	int num_borders = epsilod_num_borders(dims);

	// Initialize
	for (int i = 0; i < num_borders; i++) {
		merged_indexes[i] = i;
	}

	for (int i = 0; i < num_borders; i++) {
		if (hit_shapeCmp(shp_border[i], HIT_SHAPE_NULL))
			continue;
		// Brute force
		for (int j = 0; j < num_borders; j++) {
			// Skip current border or borders that already merge to another
			// This prevents index loops
			if (i == j || merged_indexes[j] != j)
				continue;
			// Check if shp_border[i] is contained in shp_border[j]
			HitShape shp_int = hit_shapeIntersect(shp_border[i], shp_border[j]);
			if (hit_shapeCmp(shp_border[i], shp_int)) {
				bool keep_contiguity = true;
				// Check if last n-1 dim cards match
				for (int d = 1; d < dims; d++) {
					if (hit_shapeSigCard(shp_border[i], d) != hit_shapeSigCard(shp_border[j], d)) {
						keep_contiguity = false;
						break;
					}
				}
				if (keep_contiguity) {
					merged_indexes[i] = j;
					// A tile may only be merged once
					break;
				}
			}
		}
	}

	// Avoid chained indexes
	for (int i = 0; i < num_borders; i++) {
		int idx;
		int idx_aux = i;
		do {
			idx     = idx_aux;
			idx_aux = merged_indexes[idx];
		} while (idx != idx_aux);
		merged_indexes[i] = idx;
	}
}

// TODO @seralpa these need better names
/**
 * @brief Packs the necessary data to work out global data coordinates from local thread indexes within a tile and the stencil's border sizes.
 * @param tile The tile taken as local reference.
 * @param borders Border sizes.
 * @return
 */
EpsilodCoords build_coords(HitTile(EPSILOD_BASE_TYPE) tile, EpsilodBorders borders) {
	HitTile *p_global_mat = hit_tileRoot(&tile);

	EpsilodCoords coords = {0};
	coords.dims          = hit_tileDims(*p_global_mat);
	for (int i = 0; i < coords.dims; i++) {
		coords.size[i]   = hit_tileDimCard(*p_global_mat, i);
		coords.offset[i] = hit_shapeSig(tile.shape, i).begin;
	}
	coords.borders = borders;

	return coords;
}

void free_epsilod_tiles(EpsilodTiles *p_tiles) {
	int dims = hit_tileDims(p_tiles->mat);
	Ctrl_Free(NULL, p_tiles->mat, p_tiles->inner, p_tiles->io);
	if (epsilod_align() == EPSILOD_MEM_ALIGN_THREADS && dims > 1) {
		Ctrl_Free(NULL, p_tiles->inner_compute);
	}

	for (int i = 0; i < epsilod_num_borders(dims); i++) {
		Ctrl_Free(NULL, p_tiles->border_in[i], p_tiles->border_out[i]);
		if (comms_contiguous_buffers()) {
			Ctrl_Free(NULL, p_tiles->cont_border_in[i], p_tiles->cont_border_out[i]);
			Ctrl_Free(NULL, p_tiles->comms_border_in[i], p_tiles->comms_border_out[i]);
		}
	}
	for (int i = 0; i < dims; i++) {
		Ctrl_Free(NULL, p_tiles->border_out_dev[i][0], p_tiles->border_out_dev[i][1]);
	}
	hit_patternFree(&(p_tiles->neighSync));

	if (comms_contiguous_buffers()) {
		free(p_tiles->cont_border_in);
		free(p_tiles->cont_border_out);
	}
	free(p_tiles->border_in);
	free(p_tiles->border_out);
	free(p_tiles->comms_border_in);
	free(p_tiles->comms_border_out);
	free(p_tiles->border_out_dev);

	free(p_tiles);
}

/**
 * @brief Sets borders status.
 * Borders are marked as active based on stencil weights.
 * The contents of border status pointers are overwritten.
 * @param[inout] comm_args Communication data to update.
 * @param stencil A tile containing stencil weights.
 */
void set_active_borders_bystencil(EpsilodCommArgs comm_args, HitTile_float stencil) {
	bool *border_in_active  = comm_args.border_in_active;
	bool *border_out_active = comm_args.border_out_active;

	int dims        = hit_tileDims(stencil);
	int num_borders = epsilod_num_borders(dims);

	for (int i = 0; i < num_borders; i++) {
		border_in_active[i] = false;
	}

	// Traverse the stencil to detect active and inactive borders due to weights
	int indexes[dims];
	int displacement = 0;
	for (int j = 0; j < dims; j++)
		indexes[j] = 0;
	bool end_analysis = false;
	while (!end_analysis) {
		// Check if there is a weight in the stencil position
		if (hit(stencil, displacement) != 0) {
			// Active border, compute its number to raise the flag
			int acum   = 1;
			int border = 0;
			for (int j = dims - 1; j >= 0; j--) {
				// -hit_tileDimBegin(stencil, j) -> size of low border
				if (indexes[j] > -hit_tileDimBegin(stencil, j))
					border += 2 * acum;
				else if (indexes[j] == -hit_tileDimBegin(stencil, j))
					border += acum;
				acum *= 3;
			}
			border_in_active[border] = true;
		}
		// Advance to the next stencil position
		displacement++;
		for (int k = dims - 1; k >= 0; k--) {
			indexes[k]++;
			if (k == 0 && indexes[0] == hit_tileDimCard(stencil, 0))
				end_analysis = true;
			if (indexes[k] >= hit_tileDimCard(stencil, k))
				indexes[k] = 0;
			else
				break;
		}
	}
	// Always skip false border: tile inner
	border_in_active[num_borders / 2] = false;

	// Outbound border state is the same as inbound.
	for (int i = 0; i < num_borders; i++) {
		border_out_active[i] = border_in_active[i];
	}
}

/**
 * @brief Generates neighbour processor coordinate displacements (shifts).
 * This function expects border status based on stencil data
 * @param[inout] comm_args Communications related data to update.
 * @param lay HitLayout
 */
void set_shifts(EpsilodCommArgs comm_args, HitLayout lay) {
	HitRanks *p_shifts_in  = comm_args.shifts_in;
	HitRanks *p_shifts_out = comm_args.shifts_out;

	int dims = hit_layNumDims(lay);

	// Build the neighbor shifts
	for (int i = 0; i < epsilod_num_borders(dims); i++) {
		p_shifts_in[i]  = HIT_RANKS_NULL;
		p_shifts_out[i] = HIT_RANKS_NULL;

		// TODO @davdiez Review: HIT_RANKS_NULL makes "null" HitRanks valid shifts, though they are not used
		// Non-active borders, null ranks
		if (!comm_args.border_in_active[i])
			continue;

		// Extract ranks for this border
		int digits = i;
		for (int j = dims - 1; j >= 0; j--) {
			p_shifts_in[i].rank[j]  = digits % 3 - 1;
			p_shifts_out[i].rank[j] = -p_shifts_in[i].rank[j];
			digits /= 3;
		}
	}
}

/**
 * @brief Marks borders as inactive based on the absence of neighbouring processes.
 * @param[inout] p_border_in_active Array to update indicating if borders are active.
 * @param lay The layout used to query neighbour presence.
 * @param shifts Displacements to neighbors
 */
void deactivate_empty_neighbors(bool *p_border_active, HitLayout lay, HitRanks *shifts) {

	for (int i = 0; i < epsilod_num_borders(hit_layNumDims(lay)); i++) {
		// Skip empty borders
		if (!p_border_active[i])
			continue;

		// Deactivate borders without neighbor
		HitRanks neigh = hit_layNeighborN(lay, shifts[i]);
		if (neigh.rank[0] == HIT_RANK_NULL) {
			p_border_active[i] = false;
		}
	}
}

void init_comm_args(EpsilodCommArgs *p_comm_args, HitTile_float stencil, HitLayout lay) {

	set_active_borders_bystencil(*p_comm_args, stencil);
	set_shifts(*p_comm_args, lay);
	deactivate_empty_neighbors(p_comm_args->border_in_active, lay, p_comm_args->shifts_in);
	deactivate_empty_neighbors(p_comm_args->border_out_active, lay, p_comm_args->shifts_out);
}

EpsilodTiles *create_tiles(PCtrl comm, HitLayout lay, HitTile(EPSILOD_BASE_TYPE) * global_mat, EpsilodBorders borders, EpsilodCommArgs comm_args) {
	EpsilodTiles *p_tiles             = (EpsilodTiles *)malloc(sizeof(EpsilodTiles));
	int           dims                = hit_layNumDims(lay);
	int           num_borders         = epsilod_num_borders(dims);
	bool         *p_border_in_active  = comm_args.border_in_active;
	bool         *p_border_out_active = comm_args.border_out_active;
	HitRanks     *shifts_in           = comm_args.shifts_in;

	p_tiles->mat           = create_tile_mat(comm, global_mat, lay.shape, borders);
	p_tiles->inner         = create_tile_inner(&p_tiles->mat, lay, p_border_out_active, borders);
	p_tiles->inner_compute = create_tile_inner_compute(&p_tiles->mat, &p_tiles->inner);
	p_tiles->io            = create_tile_io(&p_tiles->mat, *global_mat, borders);

	HitShape *p_shp_border_in  = malloc(sizeof(HitShape) * num_borders);
	HitShape *p_shp_border_out = malloc(sizeof(HitShape) * num_borders);
	for (int i = 0; i < num_borders; i++) {
		p_shp_border_in[i]  = create_shape_borderin(lay.shape, p_border_in_active[i], borders, shifts_in[i]);
		p_shp_border_out[i] = create_shape_borderout(lay.shape, p_border_out_active[i], borders, shifts_in[i]);
	}

	bool contiguous = comms_contiguous_buffers();

	int      *border_in_merge_to       = malloc(sizeof(int) * num_borders);
	HitShape *p_shp_border_in_expanded = malloc(sizeof(HitShape) * num_borders);
	int      *border_out_merge_to      = malloc(sizeof(int) * num_borders);
	if (contiguous) {
		calc_borderin_merge(dims, p_shp_border_in, p_shp_border_in_expanded, border_in_merge_to);
		calc_borderout_merge(dims, p_shp_border_out, border_out_merge_to);
	}

	p_tiles->border_in  = malloc(sizeof(HitTile(EPSILOD_BASE_TYPE)) * num_borders);
	p_tiles->border_out = malloc(sizeof(HitTile(EPSILOD_BASE_TYPE)) * num_borders);
	for (int i = 0; i < num_borders; i++) {
		p_tiles->border_in[i]  = create_tile_border(&p_tiles->mat, contiguous ? p_shp_border_in_expanded[i] : p_shp_border_in[i], p_border_in_active[i] && (!contiguous || border_in_merge_to[i] == i));
		p_tiles->border_out[i] = create_tile_border(&p_tiles->mat, p_shp_border_out[i], p_border_out_active[i] && (!contiguous || border_out_merge_to[i] == i));
	}

	// Tiles used as contiguous buffers
	if (contiguous) {
		p_tiles->cont_border_in  = malloc(sizeof(HitTile(EPSILOD_BASE_TYPE)) * num_borders);
		p_tiles->cont_border_out = malloc(sizeof(HitTile(EPSILOD_BASE_TYPE)) * num_borders);
		for (int i = 0; i < num_borders; i++) {
			p_tiles->cont_border_in[i]  = create_tile_copy(comm, p_tiles->border_in[i]);
			p_tiles->cont_border_out[i] = create_tile_copy(comm, p_tiles->border_out[i]);
		}
	}

	// TODO @seralpa consider making fn build only one tile
	p_tiles->border_out_dev = malloc(sizeof(HitTile(EPSILOD_BASE_TYPE)) * dims * 2);
	create_tile_borderoutdev(p_tiles, lay, p_border_out_active, borders);

	// Tiles used in communications
	p_tiles->comms_border_in  = malloc(sizeof(HitTile(EPSILOD_BASE_TYPE)) * num_borders);
	p_tiles->comms_border_out = malloc(sizeof(HitTile(EPSILOD_BASE_TYPE)) * num_borders);
	for (int i = 0; i < num_borders; i++) {
		if (contiguous) {
			p_tiles->comms_border_in[i]  = Ctrl_Select(EPSILOD_BASE_TYPE, p_tiles->cont_border_in[border_in_merge_to[i]], p_shp_border_in[i], CTRL_SELECT_ARR_COORD);
			p_tiles->comms_border_out[i] = Ctrl_Select(EPSILOD_BASE_TYPE, p_tiles->cont_border_out[border_out_merge_to[i]], p_shp_border_out[i], CTRL_SELECT_ARR_COORD);
		} else {
			p_tiles->comms_border_in[i]  = p_tiles->border_in[i];
			p_tiles->comms_border_out[i] = p_tiles->border_out[i];
		}
	}

	free(p_shp_border_in);
	free(p_shp_border_in_expanded);
	free(p_shp_border_out);

	return p_tiles;
}

void sort_comm_indexes(EpsilodTiles tiles, CommCompIndex *p_sorted_comm_indexes) {
	int num_borders = epsilod_num_borders(hit_tileDims(tiles.mat));
	for (int i = 0; i < num_borders; i++) {
		p_sorted_comm_indexes[i] = (CommCompIndex){i, tiles.border_in[i]};
	}
	#ifdef EPSILOD_COMM_SORT
	qsort(p_sorted_comm_indexes, num_borders, sizeof(CommCompIndex), compare_comm_tiles);
	#endif
}

HitPattern create_comm_pattern(PCtrl comm, EpsilodTiles *p_tiles, EpsilodCommArgs comm_args, CommCompIndex *sorted_comm_indexes, HitLayout lay, HitType HIT_CELL) {
	HitPattern pattern           = hit_pattern(HIT_PAT_UNORDERED);
	int        num_borders       = epsilod_num_borders(hit_layNumDims(lay));
	bool      *border_in_active  = comm_args.border_in_active;
	bool      *border_out_active = comm_args.border_out_active;

	int indexCommBorderCount = 0;
	for (int j = 0, i = sorted_comm_indexes[j].index; j < num_borders; i = sorted_comm_indexes[++j].index) {

		// If both neighbors do not exist, skip adding comms
		if (!border_in_active[i] && !border_out_active[i])
			continue;

		// Use CUDA/HIP MPI aware
		if (mpi_dev_aware()) {
			if (border_in_active[i])
				(p_tiles->comms_border_in[i]).data = Ctrl_GetDevPtr(comm, p_tiles->comms_border_in[i]);
			if (border_out_active[i])
				(p_tiles->comms_border_out[i]).data = Ctrl_GetDevPtr(comm, p_tiles->comms_border_out[i]);
		}

		// Locate neighbors in the layout grid
		HitRanks neigh_in  = HIT_RANKS_NULL;
		HitRanks neigh_out = HIT_RANKS_NULL;
		if (border_in_active[i])
			neigh_in = hit_layNeighborN(lay, comm_args.shifts_in[i]);
		if (border_out_active[i])
			neigh_out = hit_layNeighborN(lay, comm_args.shifts_out[i]);

		// Add comms to the patterns
		hit_patternAdd(&pattern, hit_comSendRecv(lay, neigh_out, &(p_tiles->comms_border_out[i]), neigh_in, &(p_tiles->comms_border_in[i]), HIT_CELL));

		// Annotate the index of the border in the pattern
		comm_args.index_comm_border[indexCommBorderCount++] = i;
	}
	return pattern;
}

EpsilodGlobalCoords get_global_coords(EpsilodTiles tiles, EpsilodBorders borders) {
	EpsilodGlobalCoords g_coords = {0};
	g_coords.mat                 = build_coords(tiles.mat, borders);
	g_coords.inner               = build_coords(tiles.inner, borders);

	if (epsilod_align() == EPSILOD_MEM_ALIGN_THREADS && hit_tileDims(tiles.mat) > 1) {
		HitShape shp_mat   = tiles.mat.shape;
		HitShape shp_inner = tiles.inner.shape;
		int      last_dim  = hit_shapeDims(shp_inner) - 1;
		HitInd   inner_last_dim_offset =
			hit_shapeSig(shp_inner, last_dim).begin -
			hit_shapeSig(shp_mat, last_dim).begin;
		g_coords.inner.inner_last_dim_offset = inner_last_dim_offset;
	} else {
		g_coords.inner.inner_last_dim_offset = 0;
	}

	for (int i = 0; i < hit_tileDims(tiles.mat); i++)
		for (int j = 0; j < 2; j++)
			g_coords.border_out_dev[i][j] = build_coords(tiles.border_out_dev[i][j], borders);
	return g_coords;
}

EpsilodThreads get_threads(EpsilodTiles tiles) {
	EpsilodThreads threads = {0};
	threads.mat            = init_thread_from_tile(&tiles.mat);
	threads.inner          = init_thread_from_tile(&tiles.inner_compute);
	threads.flat           = (Ctrl_Thread){.dims = 1, .i = tiles.mat.acumCard, .j = 1, .k = 1};
	threads.touch          = (Ctrl_Thread){.dims = 1, .i = 1, .j = 0, .k = 0};

	for (int i = 0; i < hit_tileDims(tiles.mat); i++)
		for (int j = 0; j < 2; j++)
			threads.border_out_dev[i][j] = init_thread_from_tile(&tiles.border_out_dev[i][j]);

	if (comms_contiguous_buffers()) {
		int num_borders         = epsilod_num_borders(hit_tileDims(tiles.mat));
		threads.cont_border_in  = malloc(sizeof(Ctrl_Thread) * num_borders);
		threads.cont_border_out = malloc(sizeof(Ctrl_Thread) * num_borders);
		for (int i = 0; i < num_borders; i++) {
			if (!hit_tileIsNull(tiles.cont_border_in[i]))
				threads.cont_border_in[i] = init_thread_from_tile(&tiles.cont_border_in[i]);
			if (!hit_tileIsNull(tiles.cont_border_out[i]))
				threads.cont_border_out[i] = init_thread_from_tile(&tiles.cont_border_out[i]);
		}
	}

	return threads;
}

/**
 * @brief Retrieve a block suitable for computations where each device thread accesses only a single element.
 * The shape of the involved tile/s is used to assess suitable block dimensions.
 * Assigns bigger cardinalities to higher dimensions where possible.
 * @param shape The shape of the tiles that will be used
 * @return The block dimensions
 */
Ctrl_Thread get_char_from_shape(HitShape shape) {
	int MAX_BLOCK_CARD = 256;
	int dims           = hit_shapeDims(shape);
	int char_dims      = (dims > 3) ? 3 : dims;
	int acum_card      = 1;
	int block[3]       = {1, 1, 1};
	for (int dim = dims - 1, b = 0; dim >= dims - char_dims; dim--, b++) {
		// Powers of 2 from 256 down
		for (int i = 8; i >= 0; i--) {
			int size = (int)powf(2, i);
			if (hit_shapeSigCard(shape, dim) >= size && acum_card * size <= MAX_BLOCK_CARD) {
				block[b] = size;
				acum_card *= size;
				break;
			}
		}
	}

	switch (dims) {
		case 1:
			return (Ctrl_Thread){.dims = char_dims, .i = block[0], .j = 1, .k = 1};
		case 2:
			return (Ctrl_Thread){.dims = char_dims, .i = block[1], .j = block[0], .k = 1};
		default:
			return (Ctrl_Thread){.dims = char_dims, .i = block[2], .j = block[1], .k = block[0]};
	}
}

EpsilodThreads get_chars(int dims, Ctrl_Type ctrl_type, EpsilodTiles tiles) {
	/* A. Kernel characterizations */
	Ctrl_Thread char_inner[3] = {
		{.dims = 1, .i = 256, .j = 1, .k = 1},
		{.dims = 2, .i = 4, .j = 64, .k = 1},
		{.dims = 3, .i = 1, .j = 4, .k = 64}};

	Ctrl_Thread char_cpu_inner[3] = {
		{.dims = 1, .i = 1024, .j = 1, .k = 1},
		{.dims = 2, .i = 4, .j = 1024, .k = 1},
		{.dims = 3, .i = 2, .j = 2, .k = 1024}};

	Ctrl_Thread char_border[3][3] = {
		// 1D
		{{.dims = 1, .i = 256, .j = 1, .k = 1},
		 CTRL_THREAD_NULL,
		 CTRL_THREAD_NULL},
		// 2D
		{{.dims = 2, .i = 1, .j = 256, .k = 1},
		 {.dims = 2, .i = 256, .j = 1, .k = 1},
		 CTRL_THREAD_NULL},
		// 3D
		{{.dims = 3, .i = 1, .j = 4, .k = 64},
		 {.dims = 3, .i = 4, .j = 1, .k = 64},
		 {.dims = 3, .i = 4, .j = 64, .k = 1}}};

	Ctrl_Thread char_cpu_border[3][3] = {
		// 1D
		{{.dims = 1, .i = 1024, .j = 1, .k = 1},
		 CTRL_THREAD_NULL,
		 CTRL_THREAD_NULL},
		// 2D
		{{.dims = 2, .i = 1, .j = 1024, .k = 1},
		 {.dims = 2, .i = 1024, .j = 1, .k = 1},
		 CTRL_THREAD_NULL},
		// 3D
		{{.dims = 3, .i = 1, .j = 1, .k = 1024},
		 {.dims = 3, .i = 1, .j = 1, .k = 1024},
		 {.dims = 3, .i = 1, .j = 1024, .k = 1}}};

	int  char_dims = (dims > 3) ? 3 : dims;
	bool is_cpu    = ctrl_type == CTRL_TYPE_CPU;

	EpsilodThreads chars = {0};
	chars.inner          = is_cpu ? char_cpu_inner[char_dims - 1] : char_inner[char_dims - 1];
	chars.mat            = chars.inner;

	// TODO: the number of threads was reduced from 512 to 256 to support OpenCL. It should be queried instead.
	chars.flat  = (Ctrl_Thread){.dims = 1, .i = 256, .j = 1, .k = 1};
	chars.touch = (Ctrl_Thread){.dims = 1, .i = 1, .j = 1, .k = 1};

	for (int i = 0; i < dims; i++)
		for (int j = 0; j < 2; j++)
			chars.border_out_dev[i][j] = is_cpu ? char_cpu_border[char_dims - 1][(dims > 2) ? 2 : i] : char_border[char_dims - 1][(dims > 2) ? 2 : i];

	if (comms_contiguous_buffers()) {
		int num_borders       = epsilod_num_borders(dims);
		chars.cont_border_in  = malloc(sizeof(Ctrl_Thread) * num_borders);
		chars.cont_border_out = malloc(sizeof(Ctrl_Thread) * num_borders);
		for (int i = 0; i < num_borders; i++) {
			if (!hit_tileIsNull(tiles.cont_border_in[i]))
				chars.cont_border_in[i] = get_char_from_shape(tiles.cont_border_in[i].shape);
			if (!hit_tileIsNull(tiles.cont_border_out[i]))
				chars.cont_border_out[i] = get_char_from_shape(tiles.cont_border_out[i].shape);
		}
	}

	return chars;
}
