cmake_minimum_required(VERSION 3.18)
project(tsqr-tc LANGUAGES CXX CUDA)

find_package(CUDAToolkit 11.0 REQUIRED)
find_package(OpenMP)

# CUDA/CXX
foreach(lang CXX CUDA)
    set(CMAKE_${lang}_STANDARD          17)
    set(CMAKE_${lang}_STANDARD_REQUIRED ON)
    set(CMAKE_${lang}_EXTENSIONS        OFF)
endforeach()

# CUDA (1/2)
SET(CMAKE_CUDA_ARCHITECTURES 80 86)

# Directories
set(INCDIR include)
set(SRCDIR src)
set(LIBDIR lib)

file(GLOB HEADERS "${INCDIR}/gpu_monitor/*.hpp")

add_definitions(-DACC_CUDA)

##########################################################################
# Library
##########################################################################

add_library(gpu_monitor STATIC
	${SRCDIR}/gpu_monitor_cuda.cu
	${SRCDIR}/gpu_monitor_cuda.hpp
	${SRCDIR}/gpu_monitor.hpp
	${SRCDIR}/libgpu_monitor.cu
	)

target_link_libraries(gpu_monitor PRIVATE
		nvidia-ml
		)

# Include directories
target_include_directories(gpu_monitor PUBLIC ${INCDIR})
target_include_directories(gpu_monitor PUBLIC ${SRCDIR}/cutf/include)

# CUDA (2/2)
set_target_properties(gpu_monitor PROPERTIES CUDA_SEPARABLE_COMPILATION ON)

# Header
set_target_properties(gpu_monitor PROPERTIES PUBLIC_HEADER ${INCDIR}/gpu_monitor/gpu_monitor.hpp)

##########################################################################
# Bin
##########################################################################

add_executable(gpu_monitor-bin
	${SRCDIR}/gpu_monitor_cuda.cu
	${SRCDIR}/gpu_monitor_cuda.hpp
	${SRCDIR}/gpu_monitor.hpp
	${SRCDIR}/main.cpp
	)

target_link_libraries(gpu_monitor-bin PRIVATE
		nvidia-ml
		stdc++fs
		)

# Include directories
target_include_directories(gpu_monitor-bin PUBLIC ${INCDIR})
target_include_directories(gpu_monitor-bin PUBLIC ${SRCDIR}/cutf/include)

# CUDA (2/2)
set_target_properties(gpu_monitor-bin PROPERTIES CUDA_SEPARABLE_COMPILATION ON)
set_target_properties(gpu_monitor-bin PROPERTIES OUTPUT_NAME gpu_monitor)


##########################################################################
# Install
##########################################################################

INSTALL(TARGETS gpu_monitor-bin RUNTIME DESTINATION bin)
INSTALL(TARGETS gpu_monitor LIBRARY DESTINATION lib)
INSTALL(TARGETS gpu_monitor PUBLIC_HEADER DESTINATION include/gpu_monitor)
