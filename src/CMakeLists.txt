cmake_minimum_required(VERSION 3.10)


# set the project name
project(NDimensionalDem)
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin)

# policy??
cmake_policy(SET CMP0003 NEW)

### Require out-of-source builds
file(TO_CMAKE_PATH "${PROJECT_BINARY_DIR}/CMakeLists.txt" LOC_PATH)
if(EXISTS "${LOC_PATH}")
    message(FATAL_ERROR "You cannot build in a source directory (or any directory with a CMakeLists.txt file). Please make a build subdirectory. Feel free to remove CMakeCache.txt and CMakeFiles.")
endif()

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

# Documentation
option(BUILD_DOC "Build documentation" ON)
find_package(Doxygen)
if (DOXYGEN_FOUND)
    # set input and output files
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

    # request to configure the file
    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
    message("Doxygen build started")

    # note the option ALL which allows to build the docs together with the application
    add_custom_target( doc_doxygen ALL
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      COMMENT "Generating API documentation with Doxygen" )
      #VERBATIM )
else (DOXYGEN_FOUND)
  message("Doxygen need to be installed to generate the doxygen documentation")
endif (DOXYGEN_FOUND)

# add the executable
add_subdirectory(Dem)
add_subdirectory(NrrdIO-1.11.0-src)
add_subdirectory(CoarseGraining)
#add_subdirectory(Texturing)

set (max_dimension "5" CACHE STRING "Maximum simulation dimension. Higher requires more compile time, and lead to larger executable.")
add_compile_definitions(MAXDIM=${max_dimension})

if(EMSCRIPTEN)
 add_executable(DEMCGND DemCgND.cpp Dem/Parameters.cpp Dem/Tools.cpp Dem/Contacts.cpp Dem/ContactList.cpp Dem/Xml.cpp Dem/Multiproc.cpp
                        CoarseGraining/Coarsing.cpp )
 set_property(TARGET DEMCGND PROPERTY CXX_STANDARD 17)
 target_compile_options(DEMCGND PUBLIC -DSAVE_FORCE_COMPONENTS=0 -DNO_OPENMP -s USE_BOOST_HEADERS=1 ${Matlab_Macro} -O3 -Wall -Wextra -DNOTALLFORMATS)# -pthread)
 target_link_libraries(DEMCGND PUBLIC -DNO_OPENMP -O3 -lm -Wall -Wextra)# -pthread)
 set(MODULARIZE "1")
 set(EXPORT_NAME "DEMCGND")
 set_target_properties(
    DEMCGND
    PROPERTIES
    LINK_FLAGS
    "-O3 --bind -s MODULARIZE=1 -s SINGLE_FILE=1 -s EXPORT_NAME='DEMCGND' -s ASSERTIONS=1 -s ALLOW_MEMORY_GROWTH=1 -s USE_BOOST_HEADERS=1 -s USE_ZLIB=1"# --pthread -s PTHREAD_POOL_SIZE=4"# -s PROXY_TO_PTHREAD"
 )
endif()


execute_process(COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/install.sh)
