/**
 * @file epsilod.h
 * @brief Epsilod: Stencil code: Any dimensions, stencil as a pattern of weights. Data type: float
 *
 * @copyright This software is part of the EPSILOD project by Trasgo Group, UVa.
 * The relevant license, warranty and copyright notice is available in the EPSILOD project repository.
 */

#ifndef _EPSILOD_H_
#define _EPSILOD_H_

#include "epsilod_structs.h"
#include "epsilod_alb.h"
#include "epsilod_alb_heuristics.h"

/**
 * Type to communicate errors
 */
typedef int epsilod_error;
hitNewType(epsilod_error);

/**
 * Print usage: Extra options with environment variables
 */
void epsilod_print_usage();

/**
 * @brief Parform a stencil computation.
 *
 * @param sizes Sizes of the matrix to compute. in elements row major order.
 * @param stencilShape Shape of the stencil operator.
 * @param stencilData Weights of neighbours for the stencil operator. If \p f_updateCell is not null the value of non zero elements doesn't matter.
 * @param factor Divisor for neighbouring elements. Only used if \p f_updateCell is null.
 * @param numIterations Number of iterations to compute.
 * @param f_init Initialization function.
 * @param f_dev_init In-device initialization function.
 * @param f_init_copy Initialization function for the copy of the input array.
 * @param f_dev_updateCell Optional. Custom kernel to perform the stencil operation.
 * @param f_output Output function.
 * @param ext_params Extra arguments to pass to the kernel.
 * @param device_selection_file Controller device configuration file containing the information on which devices to use.
 */
void stencilComputation(HitInd                 sizes[],
						HitShape               stencilShape,
						float                  stencilData[],
						float                  factor,
						int                    numIterations,
						initDataFunction       f_init,
						initDataDeviceFunction f_dev_init,
						stencilDeviceFunction  f_init_copy,
						stencilDeviceFunction  f_dev_updateCell,
						outputDataFunction     f_output,
						Epsilod_ext           *ext_params,
						char                  *device_selection_file);

/**
 * @brief Register a init kernel.
 *
 * Creates a function named \p init_name which may be passed to \e stencilComputation as \e initDataDeviceFunction.
 *
 * @hideinitializer
 * @param init_name Name of the init kernel for the stencil.
 * @param ... List of implementations available for the kernel. In the form: ARCH, SUBARCH.
 */
#define REGISTER_INIT(init_name, ...)                                                               \
	REGISTER_INIT_N(init_name, CTRL_COUNTPARAM(__VA_ARGS__), __VA_ARGS__)                           \
	CTRL_KERNEL_CHAR(init_name, MANUAL, 64, 8, 1);                                                  \
	void init_name(PCtrl ctrl, Ctrl_Thread threads, Ctrl_Thread blockSize, int stream,              \
				   HitTile(EPSILOD_BASE_TYPE) mat, EpsilodCoords global, Epsilod_ext *ext_params) { \
		Ctrl_LaunchToStream(ctrl, init_name, threads, blockSize, stream, mat, global, *ext_params); \
	}

#define REGISTER_INIT_N(init_name, n_archs_times_2, ...)     REGISTER_INIT_N_EXP(init_name, n_archs_times_2, __VA_ARGS__)
#define REGISTER_INIT_N_EXP(init_name, n_archs_times_2, ...) INIT_PROTO(init_name, n_archs_times_2, n_archs_##n_archs_times_2, __VA_ARGS__)
#define INIT_PROTO(init_name, n_archs_times_2, n_archs, ...)                \
	CTRL_KERNEL_PROTO(init_name,                                            \
					  n_archs,                                              \
					  STENCIL_EXTRACT_ARCHS_##n_archs_times_2(__VA_ARGS__), \
					  3,                                                    \
					  OUT, HitTile(EPSILOD_BASE_TYPE), matrix,              \
					  INVAL, EpsilodCoords, global_coords,                  \
					  INVAL, Epsilod_ext, ext_params);

/**
 * @brief Register a stencil kernel.
 *
 * Creates a function named \p name which may be passed to \e stencilComputation.
 *
 * @hideinitializer
 * @param name Name of the Kernel for the stencil.
 * @param ... List of implementations available for the kernel. In the form: ARCH, SUBARCH.
 */
#define REGISTER_STENCIL(name, ...)                                               \
	REGISTER_STENCIL_N(name, CTRL_COUNTPARAM(__VA_ARGS__), __VA_ARGS__)           \
	CTRL_KERNEL_CHAR(name, MANUAL, 64, 8, 1);                                     \
	void name(PCtrl ctrl, Ctrl_Thread threads, Ctrl_Thread blockSize, int stream, \
			  HitTile(EPSILOD_BASE_TYPE) mat, HitTile(EPSILOD_BASE_TYPE) copy,    \
			  EpsilodCoords global, HitTile_float stencil, float factor,          \
			  Epsilod_ext *ext_params) {                                          \
		Ctrl_LaunchToStream(ctrl, name, threads, blockSize, stream, mat, copy,    \
							global, stencil, factor, *ext_params);                \
	}

#define REGISTER_STENCIL_N(name, n_archs_times_2, ...)     REGISTER_STENCIL_N_EXP(name, n_archs_times_2, __VA_ARGS__)
#define REGISTER_STENCIL_N_EXP(name, n_archs_times_2, ...) STENCIL_PROTO(name, n_archs_times_2, n_archs_##n_archs_times_2, __VA_ARGS__)
#define STENCIL_PROTO(name, n_archs_times_2, n_archs, ...)                  \
	CTRL_KERNEL_PROTO(name,                                                 \
					  n_archs,                                              \
					  STENCIL_EXTRACT_ARCHS_##n_archs_times_2(__VA_ARGS__), \
					  6,                                                    \
					  OUT, HitTile(EPSILOD_BASE_TYPE), matrix,              \
					  IN, HitTile(EPSILOD_BASE_TYPE), matrixCopy,           \
					  INVAL, EpsilodCoords, global_coords,                  \
					  IN, HitTile(float), stencil,                          \
					  INVAL, float, factor,                                 \
					  INVAL, Epsilod_ext, ext_params);

#define STENCIL_EXTRACT_ARCHS_2(arch, subarch)       arch, subarch
#define STENCIL_EXTRACT_ARCHS_4(arch, subarch, ...)  arch, subarch, STENCIL_EXTRACT_ARCHS_2(__VA_ARGS__)
#define STENCIL_EXTRACT_ARCHS_6(arch, subarch, ...)  arch, subarch, STENCIL_EXTRACT_ARCHS_4(__VA_ARGS__)
#define STENCIL_EXTRACT_ARCHS_8(arch, subarch, ...)  arch, subarch, STENCIL_EXTRACT_ARCHS_6(__VA_ARGS__)
#define STENCIL_EXTRACT_ARCHS_10(arch, subarch, ...) arch, subarch, STENCIL_EXTRACT_ARCHS_8(__VA_ARGS__)
#define STENCIL_EXTRACT_ARCHS_12(arch, subarch, ...) arch, subarch, STENCIL_EXTRACT_ARCHS_10(__VA_ARGS__)
#define STENCIL_EXTRACT_ARCHS_14(arch, subarch, ...) arch, subarch, STENCIL_EXTRACT_ARCHS_12(__VA_ARGS__)
#define STENCIL_EXTRACT_ARCHS_16(arch, subarch, ...) arch, subarch, STENCIL_EXTRACT_ARCHS_14(__VA_ARGS__)
#define STENCIL_EXTRACT_ARCHS_18(arch, subarch, ...) arch, subarch, STENCIL_EXTRACT_ARCHS_16(__VA_ARGS__)
#define STENCIL_EXTRACT_ARCHS_20(arch, subarch, ...) arch, subarch, STENCIL_EXTRACT_ARCHS_18(__VA_ARGS__)

#define n_archs_2  1
#define n_archs_4  2
#define n_archs_6  3
#define n_archs_8  4
#define n_archs_10 5
#define n_archs_12 6
#define n_archs_14 7
#define n_archs_16 8
#define n_archs_18 9
#define n_archs_20 10

/**
 * @brief Register a border detector for a stencil.
 *
 * This allows to specify a different kernel for the inner part of the matrix and the border.
 * @note Currently this only works for 2D stencils and FPGA kernel implementations.
 *
 * @hideinitializer
 * @param default_kernel Name of the Kernel for the internal part of the matrix.
 * @param ... Names of the Kernels for the different borders of the matrix. Up to \e EPSILOD_MAX_DIMS. non row major order :(
 */
#define REGISTER_BORDER_DETECTOR(default_kernel, ...)                  REGISTER_BORDER_DETECTOR_N(default_kernel, CTRL_COUNTPARAM(__VA_ARGS__), __VA_ARGS__)
#define REGISTER_BORDER_DETECTOR_N(default_kernel, n_borders, ...)     REGISTER_BORDER_DETECTOR_N_EXP(default_kernel, n_borders, __VA_ARGS__)
#define REGISTER_BORDER_DETECTOR_N_EXP(default_kernel, n_borders, ...) REGISTER_BORDER_DETECTOR_##n_borders##D(default_kernel, __VA_ARGS__)

#define REGISTER_BORDER_DETECTOR_2D(default_kernel, xborder_kernel, yborder_kernel)                    \
	void default_kernel##_multikernel(PCtrl       ctrl,                                                \
									  Ctrl_Thread threads, Ctrl_Thread blockSize, int stream,          \
									  HitTile(EPSILOD_BASE_TYPE) mat, HitTile(EPSILOD_BASE_TYPE) copy, \
									  EpsilodCoords global, HitTile_float stencil, float factor,       \
									  Epsilod_ext *ext_params) {                                       \
		Ctrl_Info info = Ctrl_GetInfo(ctrl);                                                           \
		if (!strcmp(info.type, "FPGA")) {                                                              \
			int size[2] = {                                                                            \
				hit_tileDimCard(mat, 0),                                                               \
				hit_tileDimCard(mat, 1),                                                               \
			};                                                                                         \
			int border_low[2] = {                                                                      \
				-hit_tileDimBegin(stencil, 0),                                                         \
				-hit_tileDimBegin(stencil, 1),                                                         \
			};                                                                                         \
			int border_high[2] = {                                                                     \
				hit_tileDimEnd(stencil, 0),                                                            \
				hit_tileDimEnd(stencil, 1),                                                            \
			};                                                                                         \
			if (size[1] <= border_low[1] || size[1] <= border_high[1]) {                               \
				Ctrl_LaunchToStream(ctrl, xborder_kernel, threads, blockSize, stream,                  \
									mat, copy, global, stencil, factor, *ext_params);                  \
			} else if (size[0] <= border_low[0] || size[0] <= border_high[0]) {                        \
				Ctrl_LaunchToStream(ctrl, yborder_kernel, threads, blockSize, stream,                  \
									mat, copy, global, stencil, factor, *ext_params);                  \
			} else {                                                                                   \
				Ctrl_LaunchToStream(ctrl, default_kernel, threads, blockSize, stream,                  \
									mat, copy, global, stencil, factor, *ext_params);                  \
			}                                                                                          \
		} else {                                                                                       \
			Ctrl_LaunchToStream(ctrl, default_kernel,                                                  \
								threads, blockSize, stream, mat, copy,                                 \
								global, stencil, factor, *ext_params);                                 \
		}                                                                                              \
	}
#define REGISTER_BORDER_DETECTOR_3D(default_kernel, yzBorder_kernel, xzBorder_kernel, xyBorder_kernel) \
	void default_kernel##_multikernel(PCtrl       ctrl,                                                \
									  Ctrl_Thread threads, Ctrl_Thread blockSize, int stream,          \
									  HitTile(EPSILOD_BASE_TYPE) mat, HitTile(EPSILOD_BASE_TYPE) copy, \
									  EpsilodCoords global, HitTile_float stencil, float factor,       \
									  Epsilod_ext *ext_params) {                                       \
		Ctrl_Info info = Ctrl_GetInfo(ctrl);                                                           \
		if (!strcmp(info.type, "FPGA")) {                                                              \
			int size[3] = {                                                                            \
				hit_tileDimCard(mat, 0),                                                               \
				hit_tileDimCard(mat, 1),                                                               \
				hit_tileDimCard(mat, 2),                                                               \
			};                                                                                         \
			int border_low[3] = {                                                                      \
				-hit_tileDimBegin(stencil, 0),                                                         \
				-hit_tileDimBegin(stencil, 1),                                                         \
				-hit_tileDimBegin(stencil, 2),                                                         \
			};                                                                                         \
			int border_high[3] = {                                                                     \
				hit_tileDimEnd(stencil, 0),                                                            \
				hit_tileDimEnd(stencil, 1),                                                            \
				hit_tileDimEnd(stencil, 2),                                                            \
			};                                                                                         \
			if (size[2] <= border_low[2] || size[2] <= border_high[2]) {                               \
				Ctrl_LaunchToStream(ctrl, yzBorder_kernel,                                             \
									threads, blockSize, stream, mat, copy,                             \
									global, stencil, factor, *ext_params);                             \
			} else if (size[1] <= border_low[1] || size[1] <= border_high[1]) {                        \
				Ctrl_LaunchToStream(ctrl, xzBorder_kernel,                                             \
									threads, blockSize, stream, mat, copy,                             \
									global, stencil, factor, *ext_params);                             \
			} else if (size[0] <= border_low[0] || size[0] <= border_high[0]) {                        \
				Ctrl_LaunchToStream(ctrl, xyBorder_kernel,                                             \
									threads, blockSize, stream, mat, copy,                             \
									global, stencil, factor, *ext_params);                             \
			} else {                                                                                   \
				Ctrl_LaunchToStream(ctrl, default_kernel,                                              \
									threads, blockSize, stream, mat, copy,                             \
									global, stencil, factor, *ext_params);                             \
			}                                                                                          \
		} else {                                                                                       \
			Ctrl_LaunchToStream(ctrl, default_kernel,                                                  \
								threads, blockSize, stream, mat, copy,                                 \
								global, stencil, factor, *ext_params);                                 \
		}                                                                                              \
	}
#define REGISTER_BORDER_DETECTOR_4D(default_kernel, yzwBorder_kernel, xzwBorder_kernel,                \
									xywBorder_kernel, xyzBorder_kernel)                                \
	void default_kernel##_multikernel(PCtrl       ctrl,                                                \
									  Ctrl_Thread threads, Ctrl_Thread blockSize, int stream,          \
									  HitTile(EPSILOD_BASE_TYPE) mat, HitTile(EPSILOD_BASE_TYPE) copy, \
									  EpsilodCoords global, HitTile_float stencil, float factor,       \
									  Epsilod_ext *ext_params) {                                       \
		Ctrl_Info info = Ctrl_GetInfo(ctrl);                                                           \
		if (!strcmp(info.type, "FPGA")) {                                                              \
			int size[4] = {                                                                            \
				hit_tileDimCard(mat, 0),                                                               \
				hit_tileDimCard(mat, 1),                                                               \
				hit_tileDimCard(mat, 2),                                                               \
				hit_tileDimCard(mat, 3),                                                               \
			};                                                                                         \
			int border_low[4] = {                                                                      \
				-hit_tileDimBegin(stencil, 0),                                                         \
				-hit_tileDimBegin(stencil, 1),                                                         \
				-hit_tileDimBegin(stencil, 2),                                                         \
				-hit_tileDimBegin(stencil, 3),                                                         \
			};                                                                                         \
			int border_high[4] = {                                                                     \
				hit_tileDimEnd(stencil, 0),                                                            \
				hit_tileDimEnd(stencil, 1),                                                            \
				hit_tileDimEnd(stencil, 2),                                                            \
				hit_tileDimEnd(stencil, 3),                                                            \
			};                                                                                         \
			if (size[3] <= border_low[3] || size[3] <= border_high[3]) {                               \
				Ctrl_LaunchToStream(ctrl, yzwBorder_kernel,                                            \
									threads, blockSize, stream, mat, copy,                             \
									global, stencil, factor, *ext_params);                             \
			} else if (size[2] <= border_low[2] || size[2] <= border_high[2]) {                        \
				Ctrl_LaunchToStream(ctrl, xzwBorder_kernel,                                            \
									threads, blockSize, stream, mat, copy,                             \
									global, stencil, factor, *ext_params);                             \
			} else if (size[1] <= border_low[1] || size[1] <= border_high[1]) {                        \
				Ctrl_LaunchToStream(ctrl, xywBorder_kernel,                                            \
									threads, blockSize, stream, mat, copy,                             \
									global, stencil, factor, *ext_params);                             \
			} else if (size[0] <= border_low[0] || size[0] <= border_high[0]) {                        \
				Ctrl_LaunchToStream(ctrl, xyzBorder_kernel,                                            \
									threads, blockSize, stream, mat, copy,                             \
									global, stencil, factor, *ext_params);                             \
			} else {                                                                                   \
				Ctrl_LaunchToStream(ctrl, default_kernel,                                              \
									threads, blockSize, stream, mat, copy,                             \
									global, stencil, factor, *ext_params);                             \
			}                                                                                          \
		} else {                                                                                       \
			Ctrl_LaunchToStream(ctrl, default_kernel,                                                  \
								threads, blockSize, stream, mat, copy,                                 \
								global, stencil, factor, *ext_params);                                 \
		}                                                                                              \
	}

#endif // _EPSILOD_H_
