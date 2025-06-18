#OPTION(LEES_EDWARD "Allow the use of Lees-Edward periodic boundary condition" OFF)
#if (LEES_EDWARD)
#  add_compile_definitions(LEESEDWARD)
#endif()

#option(SAVE_FORCE_COMPONENTS "Allow to dump the elastic/viscous/friction etc. part of the forces." OFF)
set (max_dimension "5" CACHE STRING "Maximum simulation dimension. Higher requires more compile time, and lead to larger executable.")
add_compile_definitions(MAXDIM=${max_dimension})


set(VALID_BUILD_TYPES "Debug" "RelWithDebInfo" "Release" "PerfTracking")
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS ${VALID_BUILD_TYPES})
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build, options are: Debug Release RelWithDebInfo." FORCE)
endif(NOT CMAKE_BUILD_TYPE)

#if(SAVE_FORCE_COMPONENTS)
#   add_definitions(-DSAVE_FORCE_COMPONENTS=1)
#else ()
#   add_definitions(-DSAVE_FORCE_COMPONENTS=0)
#endif()

add_executable(DEMND DEMND.cpp Parameters.cpp Tools.cpp Contacts.cpp ContactList.cpp Xml.cpp Multiproc.cpp)
include_directories(AFTER ${CMAKE_SOURCE_DIR}/Dem)

set_property(TARGET DEMND PROPERTY CXX_STANDARD 17)
if(EMSCRIPTEN)
 target_compile_options(DEMND PUBLIC -DNO_OPENMP -O3 -Wall -Wextra -DUSEBINDINGS -I${CMAKE_SOURCE_DIR}/Dem)
 target_link_libraries(DEMND PUBLIC -DNO_OPENMP -O3 -lm -Wall -Wextra -DUSEBINDINGS)
 set(MODULARIZE "1")
 set(EXPORT_NAME "DEMND")
 set_target_properties(
    DEMND
    PROPERTIES
    LINK_FLAGS
    "-O3 --bind -s MODULARIZE=1 -s EXPORT_NAME='DEMND' -s EXPORTED_RUNTIME_METHODS=[getValue]"
 )
else()

   if(CMAKE_BUILD_TYPE STREQUAL "Debug")
      target_compile_options(DEMND PUBLIC -fopenmp -Wall -Wextra)
      target_link_libraries(DEMND PUBLIC -fopenmp -lstdc++fs -lm -Wall -Wextra)
   elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
      target_compile_options(DEMND PUBLIC -O3 -fopenmp -Wall -Wextra)
      target_link_libraries(DEMND PUBLIC -O3 -fopenmp -lstdc++fs -lm -Wall -Wextra)
   elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
      target_compile_options(DEMND PUBLIC -O3 -fopenmp -Wall -Wextra)
      target_link_libraries(DEMND PUBLIC -O3 -fopenmp -lstdc++fs -lm -Wall -Wextra)
   elseif(CMAKE_BUILD_TYPE STREQUAL "PerfTracking")
      include_directories(AFTER "." /home/franz/intel/oneapi/vtune/2025.3/include/)
      target_compile_options(DEMND PUBLIC -O3 -fopenmp -Wall -Wextra)
      target_link_libraries(DEMND PUBLIC -O3 -fopenmp -lstdc++fs -lm -Wall -Wextra -L/home/franz/intel/oneapi/vtune/2025.3/lib64/ -littnotify)
   else()
      target_compile_options(DEMND PUBLIC -O3 -fopenmp -Wall -Wextra)
      target_link_libraries(DEMND PUBLIC -O3 -fopenmp -lstdc++fs -lm -Wall -Wextra)
   endif()

   

endif()
