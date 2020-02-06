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

# policy??
cmake_policy(SET CMP0003 NEW)

# find boost
find_package(Boost)
if (Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
endif(Boost_FOUND)

# Checking for Matlab
if (DEFINED ENV{Matlab_ROOT_DIR})
    set(Matlab_ROOT_DIR $ENV{Matlab_ROOT_DIR})
endif()

find_package(Matlab COMPONENTS MAT_LIBRARY)
if (Matlab_FOUND)
  set(Matlab_Macro -DMATLAB)
  set(Matlab_MX_LIBRARY -lmx)
  include_directories(${Matlab_INCLUDE_DIRS})
link_libraries(${Matlab_LIBRARIES} ${Matlab_MX_LIBRARY})
endif()

# add the executable
add_subdirectory(Dem)
add_subdirectory(NrrdIO-1.11.0-src)
add_subdirectory(CoarseGraining)
add_subdirectory(Texturing)
