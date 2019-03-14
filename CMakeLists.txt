cmake_minimum_required(VERSION 3.13)
project(MI_PDP)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fopenmp -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp -Wall")

add_executable(MI_PDP main.cpp solver.cpp solver.h pole.cpp solution.cpp)