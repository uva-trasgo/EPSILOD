# Function to compile and application/example/benchmark using the appropriate base type
function(add_app name type include libs)
	message(STATUS "Generating app ${name} for type ${type}")
	if(SUPPORT_CUDA)
		add_library(${name}_kernels_${type} OBJECT ${name}/${name}_kernels.cu)
	elseif(SUPPORT_HIP)
		add_library(${name}_kernels_${type} OBJECT ${name}/${name}_kernels.cpp)
	else(SUPPORT_CUDA)
		add_library(${name}_kernels_${type} OBJECT ${name}/${name}_kernels.c)
	endif(SUPPORT_CUDA)
	add_executable(
		${name}
		${name}/${name}.c
		$<TARGET_OBJECTS:epsilod_${type}>
		$<TARGET_OBJECTS:epsilod_kernels_${type}>
		$<TARGET_OBJECTS:${name}_kernels_${type}>
	)

	target_compile_definitions(${name} PRIVATE EPSILOD_BASE_TYPE=${type} EPSILOD_TYPES_INCLUDE=${include})
	target_compile_definitions(${name}_kernels_${type} PRIVATE EPSILOD_BASE_TYPE=${type} EPSILOD_TYPES_INCLUDE=${include})
	target_include_directories(${name} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/${name})
	target_include_directories(${name}_kernels_${type} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/${name})
	target_link_libraries(${name} ${EPSILOD_LIBS} ${libs})

	# Compile FPGA kernels for this app
	if(SUPPORT_FPGA)
		message(STATUS "Generating app ${name} FPGA kernels for type ${type}")
		compile_fpga_kernels(${name} ${type} ${include} ${name})
	endif(SUPPORT_FPGA)
endfunction()
# Epsilod versions for different types
add_epsilod_version(cell_t "${CMAKE_CURRENT_SOURCE_DIR}/gassimulation/gassimulation_types.h")

# Tests, examples, benchmarks, and applications
add_app(gassimulation cell_t "gassimulation_types.h" "")
add_app(gaussian ${EXAMPLES_BASE_TYPE} "epsilod_types.h" "")
add_app(poisson ${EXAMPLES_BASE_TYPE} "poisson_types.h" "")
add_app(laplace ${EXAMPLES_BASE_TYPE} "laplace_types.h" "")
add_app(wavesim ${EXAMPLES_BASE_TYPE} "wavesim_types.h" "")
add_app(basic_stencils float "epsilod_types.h" "")
