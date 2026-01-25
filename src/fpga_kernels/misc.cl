/**
 * @file fpga_kernels/epsilod/misc.cl
 * @brief Epsilod: Kernels for miscellaneous tasks.
 *
 * @copyright This software is part of the EPSILOD project by Trasgo Group, UVa.
 * The relevant license, warranty and copyright notice is available in the EPSILOD project repository.
 */

#include "epsilod_fpga_kernels.h"

/* COPY KERNEL FOR DEVICE INITIALIZATION */
__CTRL_FPGA_KERNEL_OPTIMIZE_FOR(8, 256, 1, 1)
__CTRL_FPGA_KERNEL_REPLICATE(4)
CTRL_KERNEL_FN(epsilod_dev_copy, FPGA, NDRANGE,
			   KHitTileR_arg(EPSILOD_BASE_TYPE, matrix), KHitTileR_arg(EPSILOD_BASE_TYPE, matrix_out)) {
	hit(matrix_out, thr_i) = hit(matrix, thr_i);

	CTRL_KERNEL_END();
}

/* EMPTY KERNEL: TO SIGNAL SUBSELECTION AND ROOT TILES AS MODIFIED TO TRACK DEPENDENCIES */
CTRL_KERNEL_FN(epsilod_dev_touch, FPGA, TASK, KHitTileR_arg(EPSILOD_BASE_TYPE, matrix)) {
	CTRL_KERNEL_END();
}
