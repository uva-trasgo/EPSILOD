/**
 * @file epsilod_kernels.h
 * @brief Epsilod: Decalaration chain for epsilod types for kernels
 *
 * @copyright This software is part of the EPSILOD project by Trasgo Group, UVa.
 * The relevant license, warranty and copyright notice is available in the EPSILOD project repository.
 */

#ifndef _EPSILOD_KERNELS_H_
#define _EPSILOD_KERNELS_H_

#define STR(a)  STR2(a)
#define STR2(a) #a

#include STR(EPSILOD_TYPES_INCLUDE)
#include "epsilod_types.h"
#ifndef EPSILOD_FPGA_KERNELS
#include "Ctrl.h"
#else // EPSILOD_FPGA_KERNELS
#include "Ctrl_FPGA_Kernel.h"
#endif // !EPSILOD_FPGA_KERNELS
#if !EPSILOD_IS_FLOAT(EPSILOD_BASE_TYPE)
Ctrl_NewType(float);
#endif // !EPSILOD_IS_FLOAT(EPSILOD_BASE_TYPE)
Ctrl_NewType(EPSILOD_BASE_TYPE);

#define EPSILOD_KERNEL(name, type, subtype, ...)                                       \
	CTRL_KERNEL(name, type, subtype, CTRL_KERNEL_EXTRACT_ARGS(__VA_ARGS__), {          \
		if (global_coords.dims == 1 ||                                                 \
			global_coords.dims == 2 && thr_j >= global_coords.inner_last_dim_offset || \
			global_coords.dims == 3 && thr_k >= global_coords.inner_last_dim_offset || \
			global_coords.dims == 4 && thr_k >= global_coords.inner_last_dim_offset) { \
			CTRL_KERNEL_EXTRACT_KERNEL_NO_STR(__VA_ARGS__)                             \
		}                                                                              \
	});                                                                                \
	CTRL_KERNEL(name##_unaligned, type, subtype, __VA_ARGS__);

#endif // _EPSILOD_KERNELS_H_
