cmake_minimum_required(VERSION 3.10)
project(main VERSION 0.1.0)

FIND_PACKAGE(OpenMP REQUIRED)

set(CMAKE_CXX_STANDARD 17)

if(OPENMP_FOUND)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

add_subdirectory(src)
add_subdirectory(scripts)
add_subdirectory(tests)
