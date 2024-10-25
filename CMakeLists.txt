

project(main)

cmake_minimum_required(VERSION 3.22)

#set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_STANDARD 17)

if($ENV{VERBOSE})
    MESSAGE(STATUS "Benchmark GEMM")
endif()


find_package(BLAS)
find_package(LAPACK)
find_package(hip)


if($ENV{VERBOSE})
	    MESSAGE(STATUS "GEMM -- BLAS_LIBRARIES   : ${BLAS_LIBRARIES}")
	    MESSAGE(STATUS "GEMM -- BLAS_LINKER_FLAGS   : ${BLAS_LINKER_FLAGS}")
	    MESSAGE(STATUS "GEMM -- LAPACK_LIBRARIES   : ${LAPACK_LIBRARIES}")
	    MESSAGE(STATUS "GEMM -- LAPACK_LINKER_FLAGS   : ${LAPACK_LINKER_FLAGS}")
endif()


MESSAGE("{COMPILE_WITH_HIP}")

include_directories(${HIP_PATH}/include)
include_directories(/opt/rocm-5.7.2/hip/include)
include_directories(/opt/rocm-5.7.2/hipblas/include)
include_directories(/opt/rocm-5.7.2/hipblas/lib)
#include_directories(/nvme0/lemoinep/soft/rocThrust/include)

include_directories(/opt/rocm-5.7.2/roctracer/include)

list(APPEND CMAKE_PREFIX_PATH /opt/rocm-5.7.2 /opt/rocm-5.7.2)

# Set the GPU to compile for
#set(GPU_TARGETS "gfx900:gfx906:gfx1032;gfx1035" CACHE STRING "GPU targets to compile for")

set(GPU_RUNTIME "HIP" CACHE STRING "Switches between HIP and CUDA")
set(GPU_RUNTIMES "HIP" "CUDA")
set_property(CACHE GPU_RUNTIME PROPERTY STRINGS ${GPU_RUNTIMES})

if(NOT "${GPU_RUNTIME}" IN_LIST GPU_RUNTIMES)
    set(ERROR_MESSAGE "GPU_RUNTIME is set to \"${GPU_RUNTIME}\".\nGPU_RUNTIME must be either HIP or CUDA.")
    message(FATAL_ERROR ${ERROR_MESSAGE})
endif()

enable_language(${GPU_RUNTIME})

set(CMAKE_${GPU_RUNTIME}_STANDARD 17)
set(CMAKE_${GPU_RUNTIME}_EXTENSIONS OFF)
set(CMAKE_${GPU_RUNTIME}_STANDARD_REQUIRED ON)

set(ROCM_ROOT "/opt/rocm-5.7.2" CACHE PATH "Root directory of the ROCm installation")
list(APPEND CMAKE_PREFIX_PATH "${ROCM_ROOT}")



find_path(ROCM_SMI_INCLUDE_DIR rocm_smi/rocm_smi.h
    PATHS ${ROCM_PATH}/include
    NO_DEFAULT_PATH
)
find_library(ROCM_SMI_LIBRARY
    NAMES rocm_smi64
    PATHS ${ROCM_PATH}/lib ${ROCM_PATH}/lib64
    NO_DEFAULT_PATH
)

if(NOT ROCM_SMI_INCLUDE_DIR OR NOT ROCM_SMI_LIBRARY)
    message(FATAL_ERROR "ROCM_SMI not found. Please ensure ROCm is installed and ROCM_PATH is set correctly.")
endif()


message(CURRENT DIR= [${CMAKE_CURRENT_SOURCE_DIR}])
message(CURRENT DIR PREFIX_PATH= [${MAKE_PREFIX_PATH}])



if(GPU_RUNTIME STREQUAL "HIP")
    list(APPEND include_dirs "${ROCM_ROOT}/include")
    MESSAGE(${ROCM_ROOT}/include)
endif()

set_source_files_properties(main.cpp PROPERTIES LANGUAGE ${GPU_RUNTIME})



add_executable(main main.cpp)

target_include_directories(main PRIVATE ${ROCM_SMI_INCLUDE_DIR})

target_link_libraries(main PRIVATE ${ROCM_SMI_LIBRARY})

target_compile_options(main PRIVATE -Wall -Wextra)








