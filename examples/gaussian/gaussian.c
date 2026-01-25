/**
 * @file gaussian.c
 * @brief Example for gaussian blus filter.
 *
 * @copyright This software is part of the EPSILOD project by Trasgo Group, UVa.
 * The relevant license, warranty and copyright notice is available in the EPSILOD project repository.
 *
 * @copyright This file is part of a modified version of a Muesli example. Thus the following applies:
 * @copyright Copyright (c) 2016
 * 		Steffen Ernsting <s.ernsting@uni-muenster.de>,
 * 		Herbert Kuchen <kuchen@uni-muenster.de>,
 * 		Fabian Wrede <fabian.wrede@wi.uni-muenster.de>.
 *
 * @copyright Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * @copyright The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 */

#include <stdio.h>
#include <stdlib.h>

#include <gaussian_ext_type.h>
#include <epsilod.h>

char  *input_file_name;
char  *output_file_name;
HitInd rows, cols;
int    max_color;

/* A.1. READ PGM FILE HEADERS */
bool readPGMHeaders(FILE *finput) {
	/* CHECK MAGIC NUMBER */
	char  line[1024];
	char *check = fgets(line, 1024, finput);
	if (check == NULL) {
		fprintf(stderr, "\nError: Input file does not contain lines: %s\n", input_file_name);
		Ctrl_Finalize();
		exit(EXIT_FAILURE);
	}
	// P2: Magic for PGM ascii format
	// P5: Magic for PGM binary format
	bool ascii = false;
	if (line[0] == 'P' && line[1] == '2')
		ascii = true;
	else if (line[0] != 'P' || line[1] != '5') {
		fprintf(stderr, "\nError: Input file %s contains an unknown magic number: %s\n", input_file_name, line);
		Ctrl_Finalize();
		exit(EXIT_FAILURE);
	}

	/* SKIP COMMENTS */
	do {
		check = fgets(line, 1024, finput);
	} while (check != NULL && line[0] == '#');

	/* READ IMAGE SIZE AND MAX COLOR */
	if (check == NULL) {
		fprintf(stderr, "\nError: Input file does not contain image sizes: %s\n", input_file_name);
		Ctrl_Finalize();
		exit(EXIT_FAILURE);
	}
	int count = sscanf(line, "%ld %ld\n", &cols, &rows);
	if (count != 2) {
		fprintf(stderr, "\nError: Input file %s contains bad image sizes line: %s\n", input_file_name, line);
		Ctrl_Finalize();
		exit(EXIT_FAILURE);
	}
	check = fgets(line, 1024, finput);
	if (check == NULL) {
		fprintf(stderr, "\nError: Input file does not contain max color: %s\n", input_file_name);
		Ctrl_Finalize();
		exit(EXIT_FAILURE);
	}
	count = sscanf(line, "%d\n", &max_color);
	if (count != 1) {
		fprintf(stderr, "\nError: Input file %s contains bad max color line: %s\n", input_file_name, line);
		Ctrl_Finalize();
		exit(EXIT_FAILURE);
	}

	/* RETURN MODE */
	return ascii;
}

/* A. INITIALIZE ARRAY: READ PGM FILE */
void initData(HitTile(EPSILOD_BASE_TYPE) tileMat, EpsilodCoords global, Epsilod_ext *ext_params) {
	int kw = ext_params->kw;

	/* OPEN FILE */
	FILE *finput = fopen(input_file_name, "r");
	if (finput == NULL) {
		fprintf(stderr, "\nError: Input file cannot be opened: %s\n", input_file_name);
		Ctrl_Finalize();
		exit(EXIT_FAILURE);
	}

	/* READ HEADERS */
	bool ascii = readPGMHeaders(finput);

	/* READ IMAGE DATA */
	char  line[1024];
	char *check;
	for (HitInd i = 0; i < tileMat.acumCard; i++)
		hit(tileMat, i) = 0;
	if (ascii) {
		for (HitInd i = 0; i < rows; i++) {
			for (HitInd j = 0; j < cols; j++) {
				check = fgets(line, 1024, finput);
				if (check == NULL) {
					fprintf(stderr, "\nError: Input file %s does not contain row*columns data in ascii mode. Stopped at data line: %ld\n", input_file_name, i * cols + j);
					Ctrl_Finalize();
					exit(EXIT_FAILURE);
				}
				if (hit_sigIn(hit_tileDimSig(tileMat, 0), i + kw / 2) &&
					hit_sigIn(hit_tileDimSig(tileMat, 1), j + kw / 2)) {
					int elem;
					int count = sscanf(line, "%d\n", &elem);
					if (count != 1) {
						fprintf(stderr, "\nError: Input file %s does not contain ascii number in data line: %ld\n", input_file_name, i);
						Ctrl_Finalize();
						exit(EXIT_FAILURE);
					}
					hit(tileMat, i + kw / 2 - hit_tileDimBegin(tileMat, 0), j + kw / 2 - hit_tileDimBegin(tileMat, 1)) = elem;
				}
			}
		}
	} else { // Binary format
		for (HitInd i = 0; i < rows; i++) {
			for (HitInd j = 0; j < cols; j++) {
				char elem;
				int  count = fread(&elem, sizeof(char), 1, finput);
				if (count != 1) {
					fprintf(stderr, "\nError: Input file %s does not contain row*columns data in binary mode. Stopped at data point: %ld\n", input_file_name, i * cols + j);
					Ctrl_Finalize();
					exit(EXIT_FAILURE);
				}
				if (hit_sigIn(hit_tileDimSig(tileMat, 0), i + kw / 2) &&
					hit_sigIn(hit_tileDimSig(tileMat, 1), j + kw / 2)) {
					hit(tileMat, i + kw / 2 - hit_tileDimBegin(tileMat, 0), j + kw / 2 - hit_tileDimBegin(tileMat, 1)) = elem;
				}
			}
		}
	}

	fclose(finput);
}

/* B. WRITE RESULTS IN PGM FILE */
void outputData(HitTile(EPSILOD_BASE_TYPE) tileMat, Epsilod_ext *ext_params) {
	int radius = ext_params->kw / 2;

	char name_headers[1024];
	char name_data[1024];

	/* HEADERS PART */
	if (hit_Rank == 0) {
		strcpy(name_headers, output_file_name);
		strcat(name_headers, "_headers");
		FILE *foutput = fopen(name_headers, "w");
		if (foutput == NULL) {
			fprintf(stderr, "\nError: Output file for headers cannot be opened: %s\n", output_file_name);
			Ctrl_Finalize();
			exit(EXIT_FAILURE);
		}

		/* WRITE HEADERS */
		fprintf(foutput, "P2\n");
		fprintf(foutput, "%ld %ld\n", cols, rows);
		fprintf(foutput, "%d\n", max_color);
		// fprintf( foutput, "# Generated by EPSILOD v1.1 from %s, kw: %d\n", input_file_name, ext_params.kw );
		fclose(foutput);
	}

	/* DATA PART */
	/* Create a virtual tile with the real image shape to use as reference */
	HitShape shp_root = hitShape((radius, rows + radius - 1), (radius, cols + radius - 1));
	HitTile(EPSILOD_BASE_TYPE) virtual_root;
	hit_tileDomainShape(&virtual_root, EPSILOD_BASE_TYPE, shp_root);
	tileMat.ref = &virtual_root;

	/* Select the local data part without borders */
	HitShape shp_data       = hitShape((radius, rows + radius - 1), (radius, cols + radius - 1));
	HitShape shp_local_data = hit_shapeIntersect(shp_data, hit_tileShape(tileMat));
	HitTile(EPSILOD_BASE_TYPE) data;
	hit_tileSelectArrayCoords(&data, &tileMat, shp_local_data);

	/* Write distributed file */
	strcpy(name_data, output_file_name);
	strcat(name_data, "_data");
	// hit_tileTextFileWrite(&data, name_data, HIT_FILE_ARRAY, HIT_FILE_FLOAT, 3, 0);
	// var, fileNamePrefix, fileNameSuffix, fileRank, format, coord, header, datatype, formatSize1, formatSize2
	#if EPSILOD_IS_FLOAT(EPSILOD_BASE_TYPE)
	int datatype = HIT_FILE_FLOAT;
	#else
	int datatype = HIT_FILE_DOUBLE;
	#endif
	hit_tileFileWriteOptions(&data, name_data, "", HIT_FILE_RUNTIME, HIT_FILE_TEXT, HIT_FILE_ARRAY, HIT_FILE_NO_HEADER, datatype, 3, 0);

	/* Join file parts */
	MPI_Barrier(MPI_COMM_WORLD);
	if (hit_Rank == 0) {
		char command[2048];
		sprintf(command, "cat %s%s %s%s > %s", output_file_name, "_headers", output_file_name, "_data", output_file_name);
		system(command);
		sprintf(command, "rm %s%s %s%s", output_file_name, "_headers", output_file_name, "_data");
		system(command);
	}
}

/* D. DECLARATIONS OF OPTIMIZED STENCIL KERNEL
 * SEE gaussian_kernel.c FILE */
REGISTER_STENCIL(updateCell_gaussian, GENERIC, DEFAULT);

/* HELP: PRINT ARGUMENT USAGE */
void printUsage(char *argv[]) {
	if (hit_Rank == 0) {
		fprintf(stderr, "\n=== EPSILOD EXAMPLE: Gaussian blur ===\n");
		fprintf(stderr, "\nUsage: %s <numIterations> <kw> <image_input_file> <image_output_file> <device_selection_file>\n", argv[0]);
		fprintf(stderr, "\n");
	}
}

/* MAIN: STENCIL PROGRAM, READ ARGUMENTS AND CALL THE PATTERN */
int main(int argc, char *argv[]) {
	/* Init communication system */
	Ctrl_Init(&argc, &argv);

	/* Check program arguments number */
	if (argc != 6) {
		printUsage(argv);
		exit(EXIT_FAILURE);
	}

	/* READ ARGUMENTS */
	int iterations  = atoi(argv[1]);
	int kw          = atoi(argv[2]);
	input_file_name = argv[3];
	char o_f_name[1024];
	sprintf(o_f_name, "%s_%d_%d.ascii.pgm", argv[4], iterations, kw);
	output_file_name            = o_f_name;
	char *device_selection_file = argv[5];

	/* STENCIL DECLARATION */
	/* RADIUS */
	int      radius               = kw / 2;
	HitShape shp_stencil_gaussian = hitShape((-radius, radius), (-radius, radius));

	/* WEIGHTS: SPECIALIZED KERNEL, VALUES ARE USED ONLY TO COMPUTE BORDERS */
	int   stencilDataSize = (2 * radius + 1) * (2 * radius + 1);
	float stencilData_gaussian[stencilDataSize];
	for (int i = 0; i < stencilDataSize; i++)
		stencilData_gaussian[i] = 1; // Non-Zero arbitrary value

	/* POINTER TO SPECIFIC KERNEL */
	stencilDeviceFunction f_stencil = updateCell_gaussian;

	/* READ SIZES FROM FILE */
	FILE *finput = fopen(input_file_name, "r");
	if (finput == NULL) {
		fprintf(stderr, "\nError: Input file cannot be opened: %s\n", input_file_name);
		Ctrl_Finalize();
		exit(EXIT_FAILURE);
	}
	readPGMHeaders(finput);
	fclose(finput);
	HitInd sizes[3] = {rows + (radius * 2), cols + (radius * 2), 0};

	/* LAUNCH STENCIL COMPUTATION */
	Epsilod_ext ext_params = {.kw = kw};
	stencilComputation(sizes, shp_stencil_gaussian, stencilData_gaussian, 1.0f, iterations, initData, NULL, NULL, f_stencil, outputData, &ext_params, device_selection_file);

	/* END */
	Ctrl_Finalize();
	return 0;
}
