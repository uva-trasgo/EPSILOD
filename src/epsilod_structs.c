/**
 * @file epsilod_structs.c
 * @brief Epsilod: separate file for struct to avoid circular references
 *
 * @copyright This software is part of the EPSILOD project by Trasgo Group, UVa.
 * The relevant license, warranty and copyright notice is available in the EPSILOD project repository.
 */

#include "epsilod_structs.h"
#include <ctype.h>

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
	Ctrl_Thread tile_threads;
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
		default:
			Ctrl_ThreadInit(tile_threads,
							hit_tileDimCard(*p_tile, 0),
							hit_tileDimCard(*p_tile, 1),
							hit_tileDimCard(*p_tile, 2));
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
	Ctrl_Alloc(comm, mat);
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
 * @brief Creates a tile spanning an inbound halo.
 * This defines data in the local tile to be received from other processes.
 *
 * @param p_mat The local tile.
 * @param shp_lay The layout shape.
 * @param active Whether the halo is active.
 * @param borders Border sizes.
 * @param shift_in Displacement to halo's neighbor.
 * @return A tile spanning an inbound halo.
 */
HitTile(EPSILOD_BASE_TYPE) create_tile_borderin(HitTile(EPSILOD_BASE_TYPE) * p_mat, HitShape shp_lay, bool active, EpsilodBorders borders, HitRanks shift_in) {
	// Non-active borders, null tiles
	if (!active) return *(HitTile(EPSILOD_BASE_TYPE) *)&HIT_TILE_NULL;
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
	HitTile(EPSILOD_BASE_TYPE) tmp = Ctrl_Select(EPSILOD_BASE_TYPE, *p_mat, shp_border_in, CTRL_SELECT_ARR_COORD);
	return tmp;
}

/**
 * @brief Creates a tile spanning an outbound border.
 * This defines data in the local tile to be sent to other processes.
 *
 * @param p_mat The local tile.
 * @param shp_lay The layout shape.
 * @param active Whether the border is active.
 * @param borders Border sizes.
 * @param shift_in Displacement to halo's neighbor.
 * @return A tile spanning an outbound border.
 */
HitTile(EPSILOD_BASE_TYPE) create_tile_borderout(HitTile(EPSILOD_BASE_TYPE) * p_mat, HitShape shp_lay, bool active, EpsilodBorders borders, HitRanks shift_in) {
	// Non-active borders, null tiles
	if (!active) return *(HitTile(EPSILOD_BASE_TYPE) *)&HIT_TILE_NULL;
	HitShape shp_border_out = shp_lay;

	// Extract ranks for this border
	for (int j = 0; j < hit_shapeDims(shp_lay); j++) {
		if (shift_in.rank[j] == -1) {
			shp_border_out = hit_shapeTransform(shp_border_out, j, HIT_SHAPE_LAST, borders.low[j]);
		} else if (shift_in.rank[j] == 1) {
			shp_border_out = hit_shapeTransform(shp_border_out, j, HIT_SHAPE_FIRST, borders.high[j]);
		}
	}
	return Ctrl_Select(EPSILOD_BASE_TYPE, *p_mat, shp_border_out, CTRL_SELECT_ARR_COORD);
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
				p_tiles->border_out_dev[i][j] = *(HitTile(EPSILOD_BASE_TYPE) *)&HIT_TILE_NULL;
			}
		}
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

void freeEpsilodTiles(EpsilodTiles *p_tiles) {
	int dims = hit_tileDims(p_tiles->mat);
	Ctrl_Free(NULL, p_tiles->mat, p_tiles->inner, p_tiles->io);

	for (int i = 0; i < epsilod_num_borders(dims); i++) {
		Ctrl_Free(NULL, p_tiles->border_in[i], p_tiles->border_out[i]);
	}
	for (int i = 0; i < dims; i++) {
		Ctrl_Free(NULL, p_tiles->border_out_dev[i][0], p_tiles->border_out_dev[i][1]);
	}
	hit_patternFree(&(p_tiles->neighSync));

	free(p_tiles->border_in);
	free(p_tiles->border_out);
	free(p_tiles->border_out_dev);

	free(p_tiles);
}

void print_all(const char *format, ...) {
	if (epsilod_exp_mode())
		return;
	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);
}

void print_once(const char *format, ...) {
	if (epsilod_exp_mode())
		return;
	if (hit_Rank == 0) {
		va_list args;
		va_start(args, format);
		vprintf(format, args);
		va_end(args);
	}
}

void print_gather(char *buffer, size_t buffer_size, const char *separator) {
	char *buffer_all   = (char *)malloc(hit_NProcs * buffer_size * sizeof(char));
	char *p_buffer_all = buffer_all;
	// TODO @davdiez use Hitmap functions (currently this operation not implemented in Hitmap)
	MPI_Gather(buffer, buffer_size, MPI_CHAR,
			   buffer_all, buffer_size, MPI_CHAR,
			   0, hit_Comm);

	if (hit_Rank == 0) {
		printf("\n");
		for (int i = 0; i < hit_NProcs; i++) {
			printf("%s%s", p_buffer_all, separator);
			p_buffer_all += buffer_size;
		}
		fflush(stdout);
	}
	free(buffer_all);
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
		if (!comm_args.border_in_active[i]) continue;

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
		if (!p_border_active[i]) continue;

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

	p_tiles->mat   = create_tile_mat(comm, global_mat, lay.shape, borders);
	p_tiles->inner = create_tile_inner(&p_tiles->mat, lay, p_border_out_active, borders);
	p_tiles->io    = create_tile_io(&p_tiles->mat, *global_mat, borders);

	p_tiles->border_in  = malloc(sizeof(HitTile(EPSILOD_BASE_TYPE)) * num_borders);
	p_tiles->border_out = malloc(sizeof(HitTile(EPSILOD_BASE_TYPE)) * num_borders);
	for (int i = 0; i < num_borders; i++) {
		p_tiles->border_in[i]  = create_tile_borderin(&p_tiles->mat, lay.shape, p_border_in_active[i], borders, shifts_in[i]);
		p_tiles->border_out[i] = create_tile_borderout(&p_tiles->mat, lay.shape, p_border_out_active[i], borders, shifts_in[i]);
	}

	// TODO @seralpa consider making fn build only one tile
	p_tiles->border_out_dev = malloc(sizeof(HitTile(EPSILOD_BASE_TYPE)) * dims * 2);
	create_tile_borderoutdev(p_tiles, lay, p_border_out_active, borders);
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
		if (!(border_in_active[i]) && !border_out_active[i])
			continue;

		// Use CUDA/HIP MPI aware
		if (mpi_dev_aware()) {
			if (border_in_active[i])
				(p_tiles->border_in[i]).data = Ctrl_GetDevPtr(comm, p_tiles->border_in[i]);
			if (border_out_active[i])
				(p_tiles->border_out[i]).data = Ctrl_GetDevPtr(comm, p_tiles->border_out[i]);
		}

		// Locate neighbors in the layout grid
		HitRanks neigh_in  = HIT_RANKS_NULL;
		HitRanks neigh_out = HIT_RANKS_NULL;
		if (border_in_active[i])
			neigh_in = hit_layNeighborN(lay, comm_args.shifts_in[i]);
		if (border_out_active[i])
			neigh_out = hit_layNeighborN(lay, comm_args.shifts_out[i]);

		// Add comms to the patterns
		hit_patternAdd(&pattern, hit_comSendRecv(lay, neigh_out, &(p_tiles->border_out[i]), neigh_in, &(p_tiles->border_in[i]), HIT_CELL));

		// Annotate the index of the border in the pattern
		comm_args.index_comm_border[indexCommBorderCount++] = i;
	}
	return pattern;
}

bool mpi_dev_aware() {
	// Read env: use CUDA/HIP MPI aware
	static int mpi_dev_aware = -1;
	if (mpi_dev_aware != -1) return mpi_dev_aware;

	mpi_dev_aware = hit_envNoYes("EPSILOD_MPI_DEV_AWARE");
	print_once("Epsilod Using Device-Aware MPI: %c\n", (mpi_dev_aware) ? 'y' : 'n');
	if (mpi_dev_aware)
		print_once(BOLD_TEXT "NOTE:" REGULAR_TEXT "Device-Aware MPI only works if it is suported and activated in the MPI layer\n");
	return mpi_dev_aware;
}

bool epsilod_exp_mode() {
	static int val = -1;
	if (val != -1)
		return val;

	val = hit_envNoYes("CTRL_EXAMPLES_EXP_MODE");
	return val;
}

bool epsilod_debug_tiles() {
	static int val = -1;
	if (val != -1)
		return val;

	val = hit_envNoYes("EPSILOD_DEBUG_TILES");
	return val;
}

EpsilodGlobalCoords get_global_coords(EpsilodTiles tiles, EpsilodBorders borders) {
	EpsilodGlobalCoords g_coords = {0};
	g_coords.mat                 = build_coords(tiles.mat, borders);
	g_coords.inner               = build_coords(tiles.inner, borders);

	for (int i = 0; i < hit_tileDims(tiles.mat); i++)
		for (int j = 0; j < 2; j++)
			g_coords.border_out_dev[i][j] = build_coords(tiles.border_out_dev[i][j], borders);
	return g_coords;
}

EpsilodThreads get_threads(EpsilodTiles tiles) {
	EpsilodThreads threads = {0};
	threads.mat            = init_thread_from_tile(&tiles.mat);
	threads.inner          = init_thread_from_tile(&tiles.inner);
	threads.flat           = (Ctrl_Thread){.dims = 1, .i = tiles.mat.acumCard, .j = 1, .k = 1};
	threads.touch          = (Ctrl_Thread){.dims = 1, .i = 1, .j = 0, .k = 0};

	for (int i = 0; i < hit_tileDims(tiles.mat); i++)
		for (int j = 0; j < 2; j++)
			threads.border_out_dev[i][j] = init_thread_from_tile(&tiles.border_out_dev[i][j]);
	return threads;
}

EpsilodThreads get_chars(int dims, Ctrl_Type ctrl_type) {
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

	for (int i = 0; i < char_dims; i++)
		for (int j = 0; j < 2; j++)
			chars.border_out_dev[i][j] = is_cpu ? char_cpu_border[char_dims - 1][i] : char_border[char_dims - 1][i];
	return chars;
}

int shape_to_str(char *buff, HitShape sh) {
	char *p_start_buffer = buff;
	buff += sprintf(buff, "[%ld:%ld:%ld",
					hit_shapeSig(sh, 0).begin, hit_shapeSig(sh, 0).end, hit_shapeSig(sh, 0).stride);
	for (int i = 1; i < hit_shapeDims(sh); i++)
		buff += sprintf(buff, ",%ld:%ld:%ld",
						hit_shapeSig(sh, i).begin, hit_shapeSig(sh, i).end, hit_shapeSig(sh, i).stride);
	buff += sprintf(buff, "] \t cards: [%ld", hit_sigCard(hit_shapeSig(sh, 0)));
	for (int i = 1; i < hit_shapeDims(sh); i++)
		buff += sprintf(buff, ",%ld", hit_sigCard(hit_shapeSig(sh, i)));
	buff += sprintf(buff, "]");
	return buff - p_start_buffer;
}

int tile_to_str(char *buffer, HitTile(EPSILOD_BASE_TYPE) * p_tile) {
	char *p_buffer       = buffer;
	char *p_start_buffer = buffer;
	p_buffer += sprintf(p_buffer, "tile ptr %p, parent %p, data %p\n  ", p_tile, p_tile->ref, p_tile->data);
	p_buffer += shape_to_str(p_buffer, p_tile->shape);
	p_buffer += sprintf(p_buffer, "\n");
	return p_buffer - p_start_buffer;
}

void dump_tiles(EpsilodTiles *p_tiles) {
	size_t buff_sz  = 100 * 1024;
	char  *buffer   = (char *)malloc(buff_sz * sizeof(char));
	char  *p_buffer = buffer;
	int    dims     = hit_tileDims(p_tiles->mat);

	p_buffer += sprintf(p_buffer, "[%d] Mat:\n  ", hit_Rank);
	p_buffer += tile_to_str(p_buffer, &p_tiles->mat);

	p_buffer += sprintf(p_buffer, "[%d] Inner:\n  ", hit_Rank);
	p_buffer += tile_to_str(p_buffer, &p_tiles->inner);

	p_buffer += sprintf(p_buffer, "[%d] IO:\n  ", hit_Rank);
	p_buffer += tile_to_str(p_buffer, &p_tiles->io);

	for (int i = 0; i < dims; i++) {
		for (int j = 0; j < 2; j++) {
			p_buffer += sprintf(p_buffer, "[%d] BorderOutDev %s dim %d:\n  ", hit_Rank, j ? "high" : "low", i);
			if (validShape(p_tiles->border_out_dev[i][j].shape))
				p_buffer += tile_to_str(p_buffer, &p_tiles->border_out_dev[i][j]);
			else
				p_buffer += sprintf(p_buffer, "None\n");
		}
	}
	for (int i = 0; i < epsilod_num_borders(dims); i++) {
		p_buffer += sprintf(p_buffer, "[%d] Border IN (%d):\n  ", hit_Rank, i);
		if (validShape(p_tiles->border_in[i].shape))
			p_buffer += tile_to_str(p_buffer, &p_tiles->border_in[i]);
		else
			p_buffer += sprintf(p_buffer, "None\n");

		p_buffer += sprintf(p_buffer, "[%d] Border OUT (%d):\n  ", hit_Rank, i);
		if (validShape(p_tiles->border_out[i].shape))
			p_buffer += tile_to_str(p_buffer, &p_tiles->border_out[i]);
		else
			p_buffer += sprintf(p_buffer, "None\n");
	}

	print_gather(buffer, buff_sz, "\n");
	free(buffer);
}

void dump_coords(EpsilodCoords coords) {
	printf("\tDims: %d\n", coords.dims);
	for (int i = 0; i < coords.dims; i++) {
		printf("\t\tSize[%d] %ld\n", i, coords.size[i]);
		printf("\t\tOffset[%d] %ld\n", i, coords.offset[i]);
	}
	printf("\tBorders:\n");
	for (int i = 0; i < EPSILOD_MAX_DIMS; i++) {
		printf("\t\tHigh[%d] %d\n", i, coords.borders.high[i]);
		printf("\t\tLow[%d] %d\n", i, coords.borders.low[i]);
	}
	fflush(stdout);
}

void dump_global_coords(EpsilodGlobalCoords g_coords) {
	printf("Mat:\n");
	fflush(stdout);
	dump_coords(g_coords.mat);
	printf("Inner:\n");
	fflush(stdout);
	dump_coords(g_coords.inner);

	for (int i = 0; i < g_coords.mat.dims; i++)
		for (int j = 0; j < 2; j++) {
			printf("Border Out Dev [%d] %s:\n", i, j ? "high" : "low");
			fflush(stdout);
			dump_coords(g_coords.border_out_dev[i][j]);
		}
}

int get_partition_dim(int dims, const char *partition_str, int start) {
	const char *part_arg = &partition_str[1];
	char       *err;
	int         dim = (int)strtol(part_arg, &err, 10);
	if (err == part_arg || dim < start || dim > dims - 1 + start) {
		fprintf(stderr, "\nError in EPSILOD_PARTITION enviroment string: Dimension should be in the range [%d:%d]. String: %s \n\n", start, dims - 1 + start, partition_str);
		MPI_Abort(MPI_COMM_WORLD, MPI_ERR_OTHER);
		exit(EXIT_FAILURE);
	}
	return dim;
}

PartitionInfo get_partition_info(int dims) {
	PartitionInfo info = {
		.type = EPSILOD_PARTITION_MULTI_DIM,
		.dims = dims,
		.dim  = -1};
	char *partition_str = getenv("EPSILOD_PARTITION");
	if (partition_str != NULL) {
		if (strlen(partition_str) > 2) {
			fprintf(stderr, "\nError in EPSILOD_PARTITION enviroment string: More than two characters. String: %s\n\n", partition_str);
			MPI_Abort(MPI_COMM_WORLD, MPI_ERR_OTHER);
			exit(EXIT_FAILURE);
		}
		switch (tolower(partition_str[0])) {
			/* Multidim partition */
			case 'm':
				info.type = EPSILOD_PARTITION_MULTI_DIM;
				// Number of dimensiones in the partition, default all
				if (partition_str[1] != '\0') {
					info.dims = get_partition_dim(dims, partition_str, 1);
				}
				break;
			/* Weighted partition in a single dimension */
			case 'w':
				info.type = EPSILOD_PARTITION_WEIGHTED;
				info.dims = 1;
				info.dim  = get_partition_dim(dims, partition_str, 0);
				break;
			/* Regular partition in a single dimension */
			case 's':
				info.type = EPSILOD_PARTITION_SINGLE_DIM;
				info.dims = 1;
				info.dim  = get_partition_dim(dims, partition_str, 0);
				break;
			/* Regular partition in all dimension except one */
			case 'n':
				info.type = EPSILOD_PARTITION_NOT_DIM;
				info.dims = dims - 1;
				info.dim  = get_partition_dim(dims, partition_str, 0);
				break;
			default:
				fprintf(stderr, "\nError in EPSILOD_PARTITION enviroment string: Unknown partition type. String: %s\n\n", partition_str);
				MPI_Abort(MPI_COMM_WORLD, MPI_ERR_OTHER);
				exit(EXIT_FAILURE);
		}
	}
	return info;
}
