void foo() {
	void launch_jacobi_kernel(real *__restrict__ const a_new, const real *__restrict__ const a,
							  real *__restrict__ const l2_norm, const int iy_start, const int iy_end,
							  const int nx, const bool calculate_norm, cudaStream_t stream);

	int main(int argc, char *argv[]) {
		MPI_CALL(MPI_Init(&argc, &argv));
		int rank;
		MPI_CALL(MPI_Comm_rank(MPI_COMM_WORLD, &rank));
		int size;
		MPI_CALL(MPI_Comm_size(MPI_COMM_WORLD, &size));
		int num_devices = 0;
		CUDA_RT_CALL(cudaGetDeviceCount(&num_devices));

		int local_rank = -1;
		{
			MPI_Comm local_comm;
			MPI_CALL(MPI_Comm_split_type(MPI_COMM_WORLD, MPI_COMM_TYPE_SHARED, rank, MPI_INFO_NULL,
										 &local_comm));

			MPI_CALL(MPI_Comm_rank(local_comm, &local_rank));

			MPI_CALL(MPI_Comm_free(&local_comm));
		}

		CUDA_RT_CALL(cudaSetDevice(local_rank % num_devices));
		CUDA_RT_CALL(cudaFree(0));

		// ny - 2 rows are distributed amongst `size` ranks in such a way
		// that each rank gets either (ny - 2) / size or (ny - 2) / size + 1 rows.
		// This optimizes load balancing when (ny - 2) % size != 0
		int chunk_size;
		int chunk_size_low  = (ny - 2) / size;
		int chunk_size_high = chunk_size_low + 1;
		// To calculate the number of ranks that need to compute an extra row,
		// the following formula is derived from this equation:
		// num_ranks_low * chunk_size_low + (size - num_ranks_low) * (chunk_size_low + 1) = ny - 2
		int num_ranks_low = size * chunk_size_low + size -
							(ny - 2); // Number of ranks with chunk_size = chunk_size_low
		if (rank < num_ranks_low)
			chunk_size = chunk_size_low;
		else
			chunk_size = chunk_size_high;

		real *a;
		CUDA_RT_CALL(cudaMalloc(&a, nx * (chunk_size + 2) * sizeof(real)));
		real *a_new;
		CUDA_RT_CALL(cudaMalloc(&a_new, nx * (chunk_size + 2) * sizeof(real)));

		CUDA_RT_CALL(cudaMemset(a, 0, nx * (chunk_size + 2) * sizeof(real)));
		CUDA_RT_CALL(cudaMemset(a_new, 0, nx * (chunk_size + 2) * sizeof(real)));

		// Calculate local domain boundaries
		int iy_start_global; // My start index in the global array
		if (rank < num_ranks_low) {
			iy_start_global = rank * chunk_size_low + 1;
		} else {
			iy_start_global =
				num_ranks_low * chunk_size_low + (rank - num_ranks_low) * chunk_size_high + 1;
		}
		int iy_end_global = iy_start_global + chunk_size - 1; // My last index in the global array

		int iy_start = 1;
		int iy_end   = iy_start + chunk_size;

		// Set diriclet boundary conditions on left and right boarder

		int leastPriority    = 0;
		int greatestPriority = leastPriority;
		CUDA_RT_CALL(cudaDeviceGetStreamPriorityRange(&leastPriority, &greatestPriority));
		cudaStream_t compute_stream;
		cudaStream_t push_top_stream;
		cudaStream_t push_bottom_stream;
		CUDA_RT_CALL(cudaStreamCreateWithPriority(&compute_stream, cudaStreamDefault, leastPriority));
		CUDA_RT_CALL(
			cudaStreamCreateWithPriority(&push_top_stream, cudaStreamDefault, greatestPriority));
		CUDA_RT_CALL(
			cudaStreamCreateWithPriority(&push_bottom_stream, cudaStreamDefault, greatestPriority));

		cudaEvent_t push_top_done;
		CUDA_RT_CALL(cudaEventCreateWithFlags(&push_top_done, cudaEventDisableTiming));
		cudaEvent_t push_bottom_done;
		CUDA_RT_CALL(cudaEventCreateWithFlags(&push_bottom_done, cudaEventDisableTiming));

		int iter = 0;

		MPI_CALL(MPI_Barrier(MPI_COMM_WORLD));
		double start = MPI_Wtime();
		PUSH_RANGE("Jacobi solve", 0)
		while (iter < iter_max) {
			CUDA_RT_CALL(cudaEventRecord(reset_l2norm_done, compute_stream));

			launch_jacobi_kernel(a_new, a, (iy_start + 1), (iy_end - 1), nx,
								 compute_stream);

			CUDA_RT_CALL(cudaStreamWaitEvent(push_top_stream, reset_l2norm_done, 0));
			launch_jacobi_kernel(a_new, a, iy_start, (iy_start + 1), nx,
								 push_top_stream);
			CUDA_RT_CALL(cudaEventRecord(push_top_done, push_top_stream));

			CUDA_RT_CALL(cudaStreamWaitEvent(push_bottom_stream, reset_l2norm_done, 0));
			launch_jacobi_kernel(a_new, a, (iy_end - 1), iy_end, nx,
								 push_bottom_stream);
			CUDA_RT_CALL(cudaEventRecord(push_bottom_done, push_bottom_stream));

			const int top    = rank > 0 ? rank - 1 : (size - 1);
			const int bottom = (rank + 1) % size;

			// Apply periodic boundary conditions
			CUDA_RT_CALL(cudaStreamSynchronize(push_top_stream));
			MPI_CALL(MPI_Sendrecv(a_new + iy_start * nx, nx, MPI_REAL_TYPE, top, 0,
								  a_new + (iy_end * nx), nx, MPI_REAL_TYPE, bottom, 0, MPI_COMM_WORLD,
								  MPI_STATUS_IGNORE));
			CUDA_RT_CALL(cudaStreamSynchronize(push_bottom_stream));
			MPI_CALL(MPI_Sendrecv(a_new + (iy_end - 1) * nx, nx, MPI_REAL_TYPE, bottom, 0, a_new, nx,
								  MPI_REAL_TYPE, top, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE));

			std::swap(a_new, a);
			iter++;
		}

		CUDA_RT_CALL(cudaEventDestroy(reset_l2norm_done));
		CUDA_RT_CALL(cudaEventDestroy(push_bottom_done));
		CUDA_RT_CALL(cudaEventDestroy(push_top_done));
		CUDA_RT_CALL(cudaStreamDestroy(push_bottom_stream));
		CUDA_RT_CALL(cudaStreamDestroy(push_top_stream));
		CUDA_RT_CALL(cudaStreamDestroy(compute_stream));

		CUDA_RT_CALL(cudaFree(a_new));
		CUDA_RT_CALL(cudaFree(a));

		MPI_CALL(MPI_Finalize());
		return 0;
	}

	template <int BLOCK_DIM_X, int BLOCK_DIM_Y>
	__global__ void jacobi_kernel(real *__restrict__ const a_new, const real *__restrict__ const a,
								  const int iy_start, const int iy_end, const int nx) {
		int iy = blockIdx.y * blockDim.y + threadIdx.y + iy_start;
		int ix = blockIdx.x * blockDim.x + threadIdx.x + 1;

		if (iy < iy_end && ix < (nx - 1)) {
			const real new_val  = 0.25 * (a[iy * nx + ix + 1] + a[iy * nx + ix - 1] +
                                         a[(iy + 1) * nx + ix] + a[(iy - 1) * nx + ix]);
			a_new[iy * nx + ix] = new_val;
		}
	}

	void launch_jacobi_kernel(real *__restrict__ const a_new, const real *__restrict__ const a,
							  const int iy_start, const int iy_end,
							  const int nx, cudaStream_t stream) {
		constexpr int dim_block_x = 32;
		constexpr int dim_block_y = 32;
		dim3          dim_grid((nx + dim_block_x - 1) / dim_block_x,
							   ((iy_end - iy_start) + dim_block_y - 1) / dim_block_y, 1);
		jacobi_kernel<dim_block_x, dim_block_y><<<dim_grid, {dim_block_x, dim_block_y, 1}, 0, stream>>>(
			a_new, a, iy_start, iy_end, nx);
		CUDA_RT_CALL(cudaGetLastError());
	}
}