cmake_minimum_required(VERSION 3.20)

project(EPSILOD LANGUAGES C CXX)

# Options

# Architectures
option(SUPPORT_CPU "Support for the CPU architecture (OpenMP backend)" ON) # Support for CPU Ctrl
option(SUPPORT_CUDA "Support for the NVIDIA GPU architecture (CUDA backend)" OFF) # Support for CUDA Ctrl
option(SUPPORT_HIP "Support for the AMD GPU architecture (HIP backend)" OFF) # Support for HIP Ctrl
option(SUPPORT_OPENCL "Support for the generic GPU architecture (OpenCL backend)" OFF) # Support for OpenCL GPU Ctrl
option(SUPPORT_FPGA "Support for the Intel FPGA architecture (Intel FPGA SDK for OpenCL backend)" OFF) # Support for OpenCL FPGA Ctrl

if(SUPPORT_HIP AND SUPPORT_CUDA)
	set(SUPPORT_HIP ON)
	set(SUPPORT_CUDA OFF)
	message(
		STATUS
		"Warning: CUDA backend compilation has been deactivated. "
		"CUDA and HIP have non-compatible declarations on their header files. "
		"This will be solved in a future release."
	)
	message(STATUS "Deafult: Compile only HIP backend.")
endif(SUPPORT_HIP AND SUPPORT_CUDA)

# Enable conditional compilation of supported architectures
if(SUPPORT_CPU)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_CTRL_ARCH_CPU_ ")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_CTRL_ARCH_CPU_ ")
	set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} -D_CTRL_ARCH_CPU_ ")
endif(SUPPORT_CPU)

if(SUPPORT_CUDA)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_CTRL_ARCH_CUDA_ ")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_CTRL_ARCH_CUDA_ ")
	set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} -D_CTRL_ARCH_CUDA_ ")
endif(SUPPORT_CUDA)

if(SUPPORT_HIP)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_CTRL_ARCH_HIP_ ")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_CTRL_ARCH_HIP_ ")
	set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} -D_CTRL_ARCH_HIP_ ")
endif(SUPPORT_HIP)

if(SUPPORT_OPENCL)
	add_compile_definitions(CL_TARGET_OPENCL_VERSION=200)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_CTRL_ARCH_OPENCL_GPU_ ")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_CTRL_ARCH_OPENCL_GPU_ ")
	set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} -D_CTRL_ARCH_OPENCL_GPU_ ")
endif(SUPPORT_OPENCL)

if(SUPPORT_FPGA)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_CTRL_ARCH_FPGA_ ")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_CTRL_ARCH_FPGA_ ")
	set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} -D_CTRL_ARCH_FPGA_ ")
endif(SUPPORT_FPGA)

# Dependencies' installation paths
set(ROCM_PATH "/opt/rocm" CACHE PATH "Path to rocm installation")
set(OPENCL_PATH "/usr/local/cuda" CACHE PATH "Path to OpenCL installation")
set(MAGMA_ROOT "/opt/magma" CACHE PATH "Path to magma installation")
set(FPGA_BOARD_PKG "/opt/intelFPGA_pro/21.2/hld/board/de10_agilex" CACHE PATH "Path to FPGA board package")

# Verbosity
set(EPSILOD_CMAKE_VERBOSE OFF) # Debug cmake
set(CMAKE_VERBOSE_MAKEFILE OFF)

# Error checking
option(CTRL_ERROR_CHECK "enable error checking" ON) # Enable error checking for cuda,opencl and hip operations
if(CTRL_ERROR_CHECK)
	set(OPENCL_GPU_ERROR_CHECK ON) # Activate OpenCL error checking
	set(CUDA_ERROR_CHECK ON) # Activate CUDA error checking
	set(HIP_ERROR_CHECK ON) # Activate HIP error checking

	set(CTRL_EXAMPLES_CUDA_ERROR_CHECK ON) # Activate CUDA error checking on reference examples
	set(CTRL_EXAMPLES_HIP_ERROR_CHECK ON) # Activate HIP error checking on reference examples
	set(CTRL_EXAMPLES_OPENCL_GPU_ERROR_CHECK ON) # Activate OpenCL error checking on reference examples
	set(CTRL_EXAMPLES_FPGA_ERROR_CHECK ON) # Activate OpenCL error checking on reference examples
endif(CTRL_ERROR_CHECK)

# Common flags and libs
if(CTRL_DEBUG)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -g3 -O0 -Wall -D_CTRL_DEBUG_ ")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g3 -O0 -Wall -D_CTRL_DEBUG_ ")
	set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} -g3 -O0 -Wall -D_CTRL_DEBUG_ ")
else(CTRL_DEBUG)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -O3 -Wall ")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wall ")
	set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} -O3 -Wall -Wno-deprecated-gpu-targets ")
endif(CTRL_DEBUG)

# External libraries
set(EPSILOD_LIBS ${EPSILOD_LIBS} -lm)

# Controllers
set(CTRL_PATH "${PROJECT_SOURCE_DIR}/extern/controllers")
set(CTRL_LIB_PATH "${CTRL_PATH}/build/Ctrl")

# Function to recompile the Controllers library with the proper backend support
function(recompile_ctrl)
	execute_process(COMMAND mkdir -p build WORKING_DIRECTORY ${CTRL_PATH})
	execute_process(
		COMMAND
			cmake -DSUPPORT_CPU:BOOL=${SUPPORT_CPU} -DSUPPORT_CUDA:BOOL=${SUPPORT_CUDA} -DSUPPORT_HIP:BOOL=${SUPPORT_HIP}
			-DSUPPORT_OPENCL:BOOL=${SUPPORT_OPENCL} -DSUPPORT_FPGA:BOOL=${SUPPORT_FPGA} -DCOMPILE_CTRL_EXAMPLES:BOOL=OFF
			-DCOMPILE_CTRL_EXAMPLES:BOOL=OFF ..
		WORKING_DIRECTORY ${CTRL_PATH}/build
	)
	execute_process(COMMAND nproc OUTPUT_VARIABLE N_PROC OUTPUT_STRIP_TRAILING_WHITESPACE)
	execute_process(COMMAND make -j ${N_PROC} WORKING_DIRECTORY ${CTRL_PATH}/build)
endfunction(recompile_ctrl)

find_library(CTRL_LIB Ctrl HINTS ${CTRL_LIB_PATH})
if(NOT CTRL_LIB)
	string(
		CONCAT
		CTRL_COMPILING_MSG
		"Controllers library not found. Compiling it for the same architectures as EPSILOD. "
		"If other options are desired, recompile it manually from ${CTRL_PATH}"
	)
	message(WARNING ${CTRL_COMPILING_MSG})
	recompile_ctrl()
	find_library(CTRL_LIB Ctrl HINTS ${CTRL_LIB_PATH})
else(NOT CTRL_LIB)
	# Check that the compiled version of the Controllers library has the proper backend support
	set(
		EPSILOD_REQUIRED_BACKENDS
		"CTRL_COMPILED_WITH_SUPPORT_FOR__CPU_${SUPPORT_CPU}_CUDA_${SUPPORT_CUDA}_HIP_${SUPPORT_HIP}_OPENCL_GPU_${SUPPORT_OPENCL}_FPGA_${SUPPORT_FPGA}"
	)
	execute_process(
		COMMAND strings libCtrl.so
		COMMAND grep CTRL_COMPILED_WITH_SUPPORT_FOR__
		OUTPUT_VARIABLE CTRL_BACKEND_SUPPORT
		WORKING_DIRECTORY ${CTRL_LIB_PATH}
		OUTPUT_STRIP_TRAILING_WHITESPACE
	)
	if(NOT EPSILOD_REQUIRED_BACKENDS STREQUAL CTRL_BACKEND_SUPPORT)
		# FIXME: This check produces false positives, when Ctrl has the backend support EPSILOD requires and more
		message(WARNING "Controllers library is compiled with incompatible backend support. Recompiling.")
		message(STATUS "Epsilod requested support ${EPSILOD_REQUIRED_BACKENDS}")
		message(STATUS "Ctrl support ${CTRL_BACKEND_SUPPORT}")
		execute_process(COMMAND rm -rf build/ WORKING_DIRECTORY ${CTRL_PATH}/build)
		recompile_ctrl()
	endif(NOT EPSILOD_REQUIRED_BACKENDS STREQUAL CTRL_BACKEND_SUPPORT)
endif(NOT CTRL_LIB)

set(CTRL_INCLUDE_PATHS ${CTRL_INCLUDE_PATHS} ${CTRL_PATH}/Ctrl/inc)

# Hitmap
set(HITMAP_PATH "${CTRL_PATH}/extern/hitmap")

find_library(HITMAP_LIB hit HINTS ${HITMAP_PATH}/lib)
if(NOT HITMAP_LIB)
	execute_process(COMMAND nproc OUTPUT_VARIABLE N_PROC)
	execute_process(COMMAND make -j ${N_PROC} WORKING_DIRECTORY ${HITMAP_PATH})
	find_library(HITMAP_LIB hit HINTS ${HITMAP_PATH}/lib)
endif(NOT HITMAP_LIB)

set(HITMAP_INCLUDE_PATHS ${HITMAP_INCLUDE_PATHS} ${HITMAP_PATH}/include)
set(HITMAP_INCLUDE_PATHS ${HITMAP_INCLUDE_PATHS} ${HITMAP_PATH}/extern/iohb1.0)
set(HITMAP_INCLUDE_PATHS ${HITMAP_INCLUDE_PATHS} ${HITMAP_PATH}/extern/matrix_io)
set(HITMAP_INCLUDE_PATHS ${HITMAP_INCLUDE_PATHS} ${HITMAP_PATH}/extern/metis/Lib)

set(EPSILOD_INCLUDE_DIRS ${EPSILOD_INCLUDE_DIRS} ${HITMAP_INCLUDE_PATHS} ${CTRL_INCLUDE_PATHS})
set(EPSILOD_LIBS ${EPSILOD_LIBS} ${HITMAP_LIB} ${CTRL_LIB})

# Version
set(VERSION_MAJOR 2)
set(VERSION_MINOR 0)
set(VERSION_REVISION 0)

# MPI
find_package(MPI REQUIRED)
if(MPI_FOUND)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DMPICH_SKIP_MPICXX -DMPI_NO_CPPBIND -DSGIMPI ")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DMPICH_SKIP_MPICXX -DMPI_NO_CPPBIND -DSGIMPI ")
	set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} -DMPICH_SKIP_MPICXX -DMPI_NO_CPPBIND -DSGIMPI ")
	set(EPSILOD_INCLUDE_DIRS ${EPSILOD_INCLUDE_DIRS} ${MPI_C_INCLUDE_DIRS})
	set(EPSILOD_LIBS ${EPSILOD_LIBS} MPI::MPI_C MPI::MPI_CXX)
endif(MPI_FOUND)

# OpenMP
find_package(OpenMP REQUIRED)
if(OPENMP_FOUND)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS} ")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS} ")
	set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} ${OpenMP_CXX_FLAGS} ")
	set(EPSILOD_INCLUDE_DIRS ${EPSILOD_INCLUDE_DIRS} ${OpenMP_C_INCLUDE_DIRS})
	if(NOT SUPPORT_HIP)
		set(EPSILOD_LIBS ${EPSILOD_LIBS} OpenMP::OpenMP_C)
	endif(NOT SUPPORT_HIP)
	# Use the Intel OMP library for much better efficiency using nested parallelism
	# set(EPSILOD_LIBS ${EPSILOD_LIBS} -liomp5)
endif(OPENMP_FOUND)

# HWLOC
if(DEFINED SKIP_HWLOC)
	message(STATUS "Skipping hwloc")
else(DEFINED SKIP_HWLOC)
	set(EPSILOD_LIBS ${EPSILOD_LIBS} "-lhwloc")
endif(DEFINED SKIP_HWLOC)

# CUDA
if(SUPPORT_CUDA)
	set(CMAKE_CUDA_HOST_COMPILER "${CMAKE_CXX_COMPILER}")
	enable_language(CUDA)
	set(CUDA_ARCH "50;72;80;89" CACHE STRING "CUDA Architectures")
	set(CMAKE_CUDA_ARCHITECTURES ${CUDA_ARCH})
	find_package(CUDAToolkit REQUIRED)
	set(EPSILOD_INCLUDE_DIRS ${EPSILOD_INCLUDE_DIRS} ${CUDAToolkit_INCLUDE_DIRS})
	set(EPSILOD_LIBS ${EPSILOD_LIBS} CUDA::cudart)
endif(SUPPORT_CUDA)

# OpenCL
if(SUPPORT_OPENCL OR SUPPORT_FPGA)
	list(APPEND CMAKE_PREFIX_PATH ${OPENCL_PATH})
	find_package(OpenCL REQUIRED)
	set(EPSILOD_INCLUDE_DIRS ${EPSILOD_INCLUDE_DIRS} ${OpenCL_INCLUDE_DIRS})
	set(EPSILOD_LIBS ${EPSILOD_LIBS} OpenCL::OpenCL)
endif(SUPPORT_OPENCL OR SUPPORT_FPGA)

# HIP
if(SUPPORT_HIP)
	set(CMAKE_CXX_COMPILER "hipcc")

	list(APPEND CMAKE_PREFIX_PATH ${ROCM_PATH})

	# AMD arquitectures to use, check if this is alredy set in HIP_OFFLOAD_ARCH
	set(HIP_OFFLOAD_ARCH "--offload-arch=gfx900 --offload-arch=gfx1100" CACHE STRING "HIP offload architectures")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${HIP_OFFLOAD_ARCH}")

	# FIXME @sergioalo hip functions return codes are ignored on all examples
	add_compile_options(-Wno-unused-result)

	# Find hip
	find_package(hip REQUIRED)
	set(EPSILOD_INCLUDE_DIRS ${EPSILOD_INCLUDE_DIRS} ${HIP_INCLUDE_DIRS})
	set(EPSILOD_LIBS ${EPSILOD_LIBS} hip::host)

	if(HIPBLAS)
		find_package(HIPBLAS REQUIRED)
		set(EPSILOD_LIBS ${EPSILOD_LIBS} roc::hipblas)
	endif(HIPBLAS)

	# Compiling for an AMD platform.
	add_definitions(-D__HIP_PLATFORM_AMD__)
endif(SUPPORT_HIP)

if(SUPPORT_FPGA)
	if(FPGA_EMULATION)
		set(FPGA_AOC_FLAGS "${FPGA_AOC_FLAGS} -report -v -g0 -march=emulator")
	else(FPGA_EMULATION)
		set(FPGA_AOC_FLAGS "${FPGA_AOC_FLAGS} -report -v -g0 -board-package=${FPGA_BOARD_PKG}")
	endif(FPGA_EMULATION)

	# Translate include directories to use on aoc
	set(AOC_EPSILOD_INCLUDES -I${PROJECT_SOURCE_DIR}/Ctrl/inc)
	foreach(INC ${EPSILOD_INCLUDE_DIRS})
		set(AOC_EPSILOD_INCLUDES ${AOC_EPSILOD_INCLUDES} -I${INC})
	endforeach()
	set(AOC_EPSILOD_DEFS -D_EPSILOD_ARCH_FPGA_)
endif(SUPPORT_FPGA)

# Verbose
if(EPSILOD_CMAKE_VERBOSE)
	message(STATUS "VERBOSE = ${CMAKE_VERBOSE_MAKEFILE}")
	message(STATUS "CPU = ${SUPPORT_CPU}")
	message(STATUS "CUDA = ${SUPPORT_CUDA}")
	message(STATUS "HIP = ${SUPPORT_HIP}")
	message(STATUS "OPENCL_GPU = ${SUPPORT_OPENCL}")
	message(STATUS "OPENCL_FPGA = ${SUPPORT_FPGA}")
	message(STATUS "C vars = ${CMAKE_C_FLAGS}")
	message(STATUS "CUDA vars = ${CMAKE_CUDA_FLAGS}")
	message(STATUS "CXX vars = ${CMAKE_CXX_FLAGS}")
	message(STATUS "INCLUDE DIRS = ${EPSILOD_INCLUDE_DIRS}")
	message(STATUS "SOURCES = ${EPSILOD_SOURCES}")
	message(STATUS "LIBS = ${EPSILOD_LIBS}")
endif(EPSILOD_CMAKE_VERBOSE)

# Apply options
if(CTRL_EXAMPLES_CUDA_ERROR_CHECK)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_CTRL_EXAMPLES_CUDA_ERROR_CHECK_ ")
	set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} -D_CTRL_EXAMPLES_CUDA_ERROR_CHECK_ ")
endif(CTRL_EXAMPLES_CUDA_ERROR_CHECK)

if(CTRL_EXPERIMENTATION_MODE)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_CTRL_EXAMPLES_EXP_MODE_ ")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_CTRL_EXAMPLES_EXP_MODE_ ")
	set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} -D_CTRL_EXAMPLES_EXP_MODE_ ")
endif(CTRL_EXPERIMENTATION_MODE)

# Add EPSILOD root path to include path for kernels
if(SUPPORT_FPGA)
	set(AOC_EPSILOD_INCLUDES ${AOC_CTRL_INCLUDES} -I${CMAKE_CURRENT_SOURCE_DIR}/src/)
endif(SUPPORT_FPGA)

# Activate ALB experiments
option(EPSILOD_ALB_EXPERIMENTATION_MODE "EPSILOD ALB experimentation output" OFF)

# ALB experiments options
if(EPSILOD_ALB_EXPERIMENTATION_MODE)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_EPS_ALB_EXP_MODE_ ")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_EPS_ALB_EXP_MODE_ ")
	set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} -D_EPS_ALB_EXP_MODE_ ")
endif(EPSILOD_ALB_EXPERIMENTATION_MODE)

# Set include paths
set(EPSILOD_INCLUDE_DIRS ${EPSILOD_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/src/)
include_directories(${EPSILOD_INCLUDE_DIRS})

# Function to compile the FPGA kernels of a given application
function(compile_fpga_kernels name type include target)
	set(FPGACC time aoc)
	set(STENCIL_FPGA_OPTS -ffp-reassociate -ffp-contract=fast -no-hardware-kernel-invocation-queue -opt-arg=-nocaching)
	set(
		STENCIL_FPGA_PREPROC_FLAGS
		${AOC_EPSILOD_DEFS}
		${AOC_EPSILOD_INCLUDES}
		-DEPSILOD_BASE_TYPE=${type}
		-DEPSILOD_TYPES_INCLUDE=${include}
	)
	set(STENCIL_FPGA_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/examples/${name}/fpga_kernels/${type})
	set(OUTPUT_DIR_CREATION mkdir -p ${STENCIL_FPGA_OUTPUT_PATH})

	file(GLOB fpga_kernels "${CMAKE_CURRENT_SOURCE_DIR}/examples/${name}/fpga_kernels/*.cl")
	foreach(kernel_src_path ${fpga_kernels})
		cmake_path(GET kernel_src_path STEM kernel_name) # Remove extension

		set(STENCIL_FPGA_OUTPUT ${STENCIL_FPGA_OUTPUT_PATH}/stencil${kernel_name}_${type}.aocx)

		add_custom_command(
			OUTPUT ${STENCIL_FPGA_OUTPUT}
			COMMAND
				${OUTPUT_DIR_CREATION} && ${FPGACC} ${FPGA_AOC_FLAGS} ${STENCIL_FPGA_OPTS} ${STENCIL_FPGA_PREPROC_FLAGS}
				${kernel_src_path} -o ${STENCIL_FPGA_OUTPUT}
			DEPENDS ${kernel_src_path} ${CMAKE_CURRENT_SOURCE_DIR}/src/${include}
		)
		add_custom_target(${kernel_name}_${type}_fpga DEPENDS ${STENCIL_FPGA_OUTPUT})
		add_dependencies(${target} ${kernel_name}_${type}_fpga)
	endforeach()
endfunction()

# Function to compile different versions of epsilod for each base type
function(add_epsilod_version type include)
	message(STATUS "Generating EPSILOD kernels for type ${type}")
	if(SUPPORT_CUDA)
		add_library(epsilod_kernels_${type} OBJECT ${CMAKE_SOURCE_DIR}/src/epsilod_kernels.cu)
	elseif(SUPPORT_HIP)
		add_library(epsilod_kernels_${type} OBJECT ${CMAKE_SOURCE_DIR}/src/epsilod_kernels.cpp)
	else(SUPPORT_CUDA)
		add_library(epsilod_kernels_${type} OBJECT ${CMAKE_SOURCE_DIR}/src/epsilod_kernels.c)
	endif(SUPPORT_CUDA)
	target_compile_definitions(epsilod_kernels_${type} PRIVATE EPSILOD_BASE_TYPE=${type} EPSILOD_TYPES_INCLUDE=${include})

	message(STATUS "Generating Epsilod for type ${type}")
	add_library(
		epsilod_${type}
		OBJECT
		${CMAKE_SOURCE_DIR}/src/epsilod_structs.c
		${CMAKE_SOURCE_DIR}/src/epsilod.c
		${CMAKE_SOURCE_DIR}/src/epsilod_alb.c
		${CMAKE_SOURCE_DIR}/src/epsilod_alb_heuristics.c
	)
	target_compile_definitions(epsilod_${type} PRIVATE EPSILOD_BASE_TYPE=${type} EPSILOD_TYPES_INCLUDE=${include})

	# Compile FPGA kernels for this version of EPSILOD
	if(SUPPORT_FPGA)
		message(STATUS "Generating EPSILOD FPGA kernels for type ${type}")
		compile_fpga_kernels(epsilod ${type} ${include} epsilod_${type})
	endif(SUPPORT_FPGA)
endfunction()

option(
	EPSILOD_USE_DOUBLE
	"Examples use double-precision floating point numbers as base type, otherwise, single-precision"
	OFF
)
if(EPSILOD_USE_DOUBLE)
	set(EXAMPLES_BASE_TYPE double)
	add_epsilod_version(double "epsilod_types.h")
else(EPSILOD_USE_DOUBLE)
	add_compile_definitions(GASSIMULATION_USE_FLOAT)
	set(EXAMPLES_BASE_TYPE float)
	add_epsilod_version(float "epsilod_types.h")
endif(EPSILOD_USE_DOUBLE)

add_subdirectory(examples)
