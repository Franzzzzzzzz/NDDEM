cmake_minimum_required(VERSION 3.10)

if(EXISTS "/opt/local/bin/g++-mp-9")
  set(CMAKE_CXX_COMPILER /opt/local/bin/g++-mp-9)
  set(CMAKE_C_COMPILER /opt/local/bin/gcc-mp-9)
else()
  set(CMAKE_CXX_COMPILER g++)
  set(CMAKE_C_COMPILER gcc)
endif()


# set the project name
project(NDimensionalDem)
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# add the executable
add_subdirectory(Dem)
add_subdirectory(NrrdIO-1.11.0-src)
add_subdirectory(CoarseGraining)
add_subdirectory(Texturing)
