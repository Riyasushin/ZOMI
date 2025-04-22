cmake_minimum_required(VERSION 3.18 FATAL_ERROR)  # Updated minimum version
project(CUDA_Freshman CXX C CUDA)

# Set supported GPU architectures (use 80 for RTX 4060)
set(CMAKE_CUDA_ARCHITECTURES 80)

# Debug flags and optimization (removed -arch)
set(CMAKE_CUDA_FLAGS "-g -G -O3")

# include_directories(./include)

add_executable(01 01.cu)