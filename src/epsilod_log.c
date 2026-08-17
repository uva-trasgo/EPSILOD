/**
 * @file epsilod_io.c
 * @brief Epsilod: Logging and information printing
 *
 * @copyright This software is part of the EPSILOD project by Trasgo Group, UVa.
 * The relevant license, warranty and copyright notice is available in the EPSILOD project repository.
 */

#include "epsilod_log.h"
#include "epsilod_env.h"
#include "../../examples/Utils/ctrl_print_info.h"

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

void print_gather(HitLayout lay, char *buffer, size_t buffer_size, const char *separator) {
	if (!hit_layImActive(lay))
		return;

	char *buffer_all = NULL;
	if (hit_Rank == 0)
		buffer_all = (char *)malloc(lay.pTopology[0]->numProcs * buffer_size * sizeof(char));
	// TODO @davdiez use Hitmap functions (currently this operation not implemented in Hitmap)
	MPI_Gather(buffer, buffer_size, MPI_CHAR,
			   buffer_all, buffer_size, MPI_CHAR,
			   0, lay.pTopology[0]->comm);

	if (hit_Rank == 0) {
		char *p_buffer_all = buffer_all;
		printf("\n");
		for (int i = 0; i < lay.pTopology[0]->numProcs; i++) {
			printf("%s%s", p_buffer_all, separator);
			p_buffer_all += buffer_size;
		}
		fflush(stdout);
		free(buffer_all);
	}
}

void print_gather_reduce(char *buffer, size_t buffer_size, const char *separator) {
	char *buffer_all = NULL;
	if (hit_Rank == 0)
		buffer_all = (char *)malloc(hit_NProcs * buffer_size * sizeof(char));
	// TODO @davdiez use Hitmap functions (currently this operation not implemented in Hitmap)
	MPI_Gather(buffer, buffer_size, MPI_CHAR,
			   buffer_all, buffer_size, MPI_CHAR,
			   0, hit_Comm);

	if (hit_Rank == 0) {
		char *p_buffer_all = buffer_all;

		typedef struct rank_range {
			int start;
			int end;
		} rank_range;
		typedef struct string_rank_range {
			char       *buffer;
			rank_range *ranges;
			int         ranges_buffer_size;
			int         ranges_size;
		} string_rank_range;

		int                unique_strings_buffer_size = hit_NProcs > 32 ? 32 : hit_NProcs;
		string_rank_range *unique_strings             = calloc(unique_strings_buffer_size, sizeof(string_rank_range));

		// Reduce
		for (int i = 0; i < hit_NProcs; i++) {
			for (int j = 0; j <= unique_strings_buffer_size; j++) {
				if (unique_strings[j].buffer == NULL) {
					if (j == unique_strings_buffer_size) {
						unique_strings_buffer_size *= 2;
						unique_strings = realloc(unique_strings, unique_strings_buffer_size * sizeof(string_rank_range));
						for (int k = unique_strings_buffer_size / 2; k < unique_strings_buffer_size; k++)
							unique_strings[k].buffer = NULL;
					}
					unique_strings[j].buffer             = p_buffer_all;
					unique_strings[j].ranges             = malloc(10 * sizeof(rank_range));
					unique_strings[j].ranges_buffer_size = 10;
					unique_strings[j].ranges_size        = 1;
					unique_strings[j].ranges[0].start    = i;
					unique_strings[j].ranges[0].end      = i;
					// A NULL is the last string in the list
					break;
				} else if (strcmp(unique_strings[j].buffer, p_buffer_all) == 0) {
					if (i - 1 == unique_strings[j].ranges[unique_strings[j].ranges_size - 1].end)
						unique_strings[j].ranges[unique_strings[j].ranges_size - 1].end++;
					else {
						unique_strings[j].ranges_size++;
						if (unique_strings[j].ranges_size >= unique_strings[j].ranges_buffer_size) {
							unique_strings[j].ranges_buffer_size *= 2;
							unique_strings[j].ranges = realloc(unique_strings[j].ranges, unique_strings[j].ranges_buffer_size * sizeof(rank_range));
						}
						unique_strings[j].ranges[unique_strings[j].ranges_size - 1].start = i;
						unique_strings[j].ranges[unique_strings[j].ranges_size - 1].end   = i;
					}
					break;
				}
			}
			p_buffer_all += buffer_size;
		}
		// Print
		printf("\n");
		for (int i = 0; i < unique_strings_buffer_size; i++) {
			if (unique_strings[i].buffer == NULL)
				break;
			char  ranges_str[1024];
			char *p_ranges_str = ranges_str;
			for (int j = 0; j < unique_strings[i].ranges_size; j++) {
				rank_range *p_range = &unique_strings[i].ranges[j];
				if (p_range->start == p_range->end)
					p_ranges_str += sprintf(p_ranges_str, "%d,", p_range->start);
				else
					p_ranges_str += sprintf(p_ranges_str, "%d-%d,",
											p_range->start,
											p_range->end);
			}
			*(--p_ranges_str) = '\0';
			printf("[%s]%s%s", ranges_str, unique_strings[i].buffer, separator);
			p_buffer_all += buffer_size;
		}
		fflush(stdout);

		// Free
		for (int i = 0; i < unique_strings_buffer_size; i++) {
			if (unique_strings[i].buffer == NULL)
				break;
			free(unique_strings[i].ranges);
		}
		free(unique_strings);
		free(buffer_all);
	}
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
	p_buffer += sprintf(p_buffer, "\n\n");
	return p_buffer - p_start_buffer;
}

int thread_to_str(char *buffer, Ctrl_Thread thread, const char prefix[]) {
	char *p_buffer       = buffer;
	char *p_start_buffer = buffer;
	p_buffer += sprintf(p_buffer, "%s%dD: (%d, %d, %d)\n", prefix, thread.dims, thread.i, thread.j, thread.k);
	return p_buffer - p_start_buffer;
}

void print_ctrl_info() {
	size_t      stdout_buffer_size = 100 * 1024;
	char        stdout_buffer[stdout_buffer_size];
	char       *p_stdout_buffer = stdout_buffer;
	const char *sep             = epsilod_exp_mode() ? "&" : "\n\n";
	p_stdout_buffer += Ctrl_SPrintInfo(p_stdout_buffer, epsilod_exp_mode());
	print_gather_reduce(stdout_buffer, stdout_buffer_size, sep);
	fflush(stdout);
}

void print_topo_info(HitTopology topo) {
	#ifdef _EPSILOD_TOPO_INFO_
	if (hit_Rank == 0) {
		char  buffer[1024];
		char *p_buffer = buffer;
		p_buffer += sprintf(p_buffer, "\nTOPOLOGY: ");
		for (int i = 0; i < hit_topDims(topo); i++) {
			p_buffer += sprintf(p_buffer, "%dx", hit_topDimCard(topo, i));
		}
		p_buffer--;
		p_buffer += sprintf(p_buffer, "\n");
		printf("%s", buffer);
		fflush(stdout);
	}
	#endif // _EPSILOD_TOPO_INFO_
}

void print_lay_info(HitLayout lay) {
	#ifdef _EPSILOD_LAY_INFO_
	size_t buff_sz  = 100 * 1024;
	char  *buffer   = (char *)malloc(buff_sz * sizeof(char));
	char  *p_buffer = buffer;

	if (hit_Rank == 0) {
		p_buffer += sprintf(p_buffer, "lay->orig: ");
		p_buffer += shape_to_str(p_buffer, lay.origShape);
		p_buffer += sprintf(p_buffer, "\n\n");
	}

	p_buffer += sprintf(p_buffer, "[%d] lay->shape: ", hit_Rank);
	p_buffer += shape_to_str(p_buffer, lay.shape);

	print_gather(lay, buffer, buff_sz, "\n");
	free(buffer);
	#endif // _EPSILOD_LAY_INFO_
}

void print_weight_info(HitWeights weights) {
	#ifdef _EPSILOD_WEIGHTS_INFO_
	if (hit_Rank == 0) {
		printf("\nPartition weights = {");
		for (int i = 0; i < weights.num_procs; i++)
			printf(" %f,", weights.ratios[i]);
		printf("\b }\n");
		fflush(stdout);
	}
	#endif // _EPSILOD_WEIGHTS_INFO_
}

void log_tiles(HitLayout lay, EpsilodTiles *p_tiles) {
	size_t buff_sz  = 100 * 1024;
	char  *buffer   = (char *)malloc(buff_sz * sizeof(char));
	char  *p_buffer = buffer;
	int    dims     = hit_tileDims(p_tiles->mat);

	p_buffer += sprintf(p_buffer, "[%d] Mat:\n  ", hit_Rank);
	p_buffer += tile_to_str(p_buffer, &p_tiles->mat);

	p_buffer += sprintf(p_buffer, "[%d] Inner:\n  ", hit_Rank);
	if (validShape(p_tiles->inner.shape))
		p_buffer += tile_to_str(p_buffer, &p_tiles->inner);
	else
		p_buffer += sprintf(p_buffer, "None\n\n");

	p_buffer += sprintf(p_buffer, "[%d] Inner compute:\n  ", hit_Rank);
	if (validShape(p_tiles->inner_compute.shape))
		p_buffer += tile_to_str(p_buffer, &p_tiles->inner_compute);
	else
		p_buffer += sprintf(p_buffer, "None\n\n");

	p_buffer += sprintf(p_buffer, "[%d] IO:\n  ", hit_Rank);
	p_buffer += tile_to_str(p_buffer, &p_tiles->io);

	for (int i = 0; i < dims; i++) {
		for (int j = 0; j < 2; j++) {
			if (validShape(p_tiles->border_out_dev[i][j].shape)) {
				p_buffer += sprintf(p_buffer, "[%d] BorderOutDev %s dim %d:\n  ", hit_Rank, j ? "high" : "low", i);
				p_buffer += tile_to_str(p_buffer, &p_tiles->border_out_dev[i][j]);
			}
		}
	}
	for (int i = 0; i < epsilod_num_borders(dims); i++) {
		if (validShape(p_tiles->border_in[i].shape)) {
			p_buffer += sprintf(p_buffer, "[%d] Border IN (%d):\n  ", hit_Rank, i);
			p_buffer += tile_to_str(p_buffer, &p_tiles->border_in[i]);
		}

		if (validShape(p_tiles->border_out[i].shape)) {
			p_buffer += sprintf(p_buffer, "[%d] Border OUT (%d):\n  ", hit_Rank, i);
			p_buffer += tile_to_str(p_buffer, &p_tiles->border_out[i]);
		}
	}

	if (comms_contiguous_buffers()) {
		for (int i = 0; i < epsilod_num_borders(dims); i++) {
			if (validShape(p_tiles->cont_border_in[i].shape)) {
				p_buffer += sprintf(p_buffer, "[%d] Buff Border IN (%d):\n  ", hit_Rank, i);
				p_buffer += tile_to_str(p_buffer, &p_tiles->cont_border_in[i]);
			}

			if (validShape(p_tiles->cont_border_out[i].shape)) {
				p_buffer += sprintf(p_buffer, "[%d] Buff Border OUT (%d):\n  ", hit_Rank, i);
				p_buffer += tile_to_str(p_buffer, &p_tiles->cont_border_out[i]);
			}
		}
	}

	for (int i = 0; i < epsilod_num_borders(dims); i++) {
		if (validShape(p_tiles->comms_border_in[i].shape)) {
			p_buffer += sprintf(p_buffer, "[%d] Comms Border IN (%d):\n  ", hit_Rank, i);
			p_buffer += tile_to_str(p_buffer, &p_tiles->comms_border_in[i]);
		}

		if (validShape(p_tiles->comms_border_out[i].shape)) {
			p_buffer += sprintf(p_buffer, "[%d] Comms Border OUT (%d):\n  ", hit_Rank, i);
			p_buffer += tile_to_str(p_buffer, &p_tiles->comms_border_out[i]);
		}
	}

	print_gather(lay, buffer, buff_sz, "\n");
	free(buffer);
}

void log_coords(EpsilodCoords coords) {
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

void log_global_coords(EpsilodGlobalCoords g_coords) {
	printf("Mat:\n");
	fflush(stdout);
	log_coords(g_coords.mat);
	printf("Inner:\n");
	fflush(stdout);
	log_coords(g_coords.inner);

	for (int i = 0; i < g_coords.mat.dims; i++)
		for (int j = 0; j < 2; j++) {
			printf("Border Out Dev [%d] %s:\n", i, j ? "high" : "low");
			fflush(stdout);
			log_coords(g_coords.border_out_dev[i][j]);
		}
}

void log_threads(HitLayout lay, const char header[], EpsilodThreads threads, EpsilodTiles *p_tiles) {

	size_t buff_sz  = 100 * 1024;
	char  *buffer   = (char *)malloc(buff_sz * sizeof(char));
	char  *p_buffer = buffer;

	char prefix[] = "\t";
	int  dims     = hit_tileDims(p_tiles->mat);

	p_buffer += sprintf(p_buffer, "[%d] %s", hit_Rank, header);
	p_buffer += sprintf(p_buffer, "%sMat: ", prefix);
	p_buffer += thread_to_str(p_buffer, threads.mat, prefix);
	p_buffer += sprintf(p_buffer, "%sInner: ", prefix);
	p_buffer += thread_to_str(p_buffer, threads.inner, prefix);
	p_buffer += sprintf(p_buffer, "%sFlat: ", prefix);
	p_buffer += thread_to_str(p_buffer, threads.flat, prefix);
	p_buffer += sprintf(p_buffer, "%sTouch: ", prefix);
	p_buffer += thread_to_str(p_buffer, threads.touch, prefix);

	for (int i = 0; i < dims; i++) {
		for (int j = 0; j < 2; j++) {
			if (hit_tileIsNull(p_tiles->border_out_dev[i][j]))
				continue;
			p_buffer += sprintf(p_buffer, "%sB. out[%d][%d]: ", prefix, i, j);
			p_buffer += thread_to_str(p_buffer, threads.border_out_dev[i][j], prefix);
		}
	}

	if (comms_contiguous_buffers()) {
		int num_borders = epsilod_num_borders(dims);
		for (int i = 0; i < num_borders; i++) {
			if (!hit_tileIsNull(p_tiles->border_in[i])) {
				p_buffer += sprintf(p_buffer, "%sB. in %d: ", prefix, i);
				p_buffer += thread_to_str(p_buffer, threads.cont_border_in[i], prefix);
			}
			if (!hit_tileIsNull(p_tiles->border_out[i])) {
				p_buffer += sprintf(p_buffer, "%sB. out %d: ", prefix, i);
				p_buffer += thread_to_str(p_buffer, threads.cont_border_out[i], prefix);
			}
		}
	}

	print_gather(lay, buffer, buff_sz, "\n");
	free(buffer);
}
