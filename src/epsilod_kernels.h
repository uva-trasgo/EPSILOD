#ifndef _EPSILOD_KERNELS_H_
#define _EPSILOD_KERNELS_H_
/**
 * @file epsilod_kernels.h
 * @brief Epsilod: Decalaration chain for epsilod types for kernels
 *
 * @copyright This software is part of the EPSILOD project by Trasgo Group, UVa.
 * The relevant license, warranty and copyright notice is available in the EPSILOD project repository.
 */

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

#endif // _EPSILOD_KERNELS_H_
