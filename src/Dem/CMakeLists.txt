#OPTION(LEES_EDWARD "Allow the use of Lees-Edward periodic boundary condition" OFF)
#if (LEES_EDWARD)
#  add_compile_definitions(LEESEDWARD)
#endif()

#option(SAVE_FORCE_COMPONENTS "Allow to dump the elastic/viscous/friction etc. part of the forces." OFF)
set (max_dimension "5" CACHE STRING "Maximum simulation dimension. Higher requires more compile time, and lead to larger executable.")
add_compile_definitions(MAXDIM=${max_dimension})

#if(SAVE_FORCE_COMPONENTS)
#   add_definitions(-DSAVE_FORCE_COMPONENTS=1)
#else ()
#   add_definitions(-DSAVE_FORCE_COMPONENTS=0)
#endif()

add_executable(DEMND DEMND.cpp Parameters.cpp Tools.cpp Contacts.cpp ContactList.cpp Xml.cpp Multiproc.cpp)
set_property(TARGET DEMND PROPERTY CXX_STANDARD 17)
if(EMSCRIPTEN)
 target_compile_options(DEMND PUBLIC -DNO_OPENMP -O3 -Wall -Wextra -DUSEBINDINGS)
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
 target_compile_options(DEMND PUBLIC -g -O3 -fopenmp -Wall -Wextra)
 target_link_libraries(DEMND PUBLIC -g -O3 -fopenmp -lstdc++fs -lm -Wall -Wextra)
endif()
