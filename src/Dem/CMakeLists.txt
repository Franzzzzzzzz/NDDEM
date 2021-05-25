add_executable(DEMND DEMND.cpp Parameters.cpp Tools.cpp Contacts.cpp ContactList.cpp Xml.cpp Multiproc.cpp)
set_property(TARGET DEMND PROPERTY CXX_STANDARD 17)
if(EMSCRIPTEN)
 target_compile_options(DEMND PUBLIC -DNO_OPENMP -g -O3 -Wall -Wextra)
 target_link_libraries(DEMND PUBLIC -DNO_OPENMP -g -O3 -lm -Wall -Wextra)
 set(MODULARIZE "1")
 set(EXPORT_NAME "DEMND")
 set_target_properties(
    DEMND
    PROPERTIES
    LINK_FLAGS
    "-O3 --bind -s MODULARIZE=1 -s EXPORT_NAME='DEMND' -s EXTRA_EXPORTED_RUNTIME_METHODS=[getValue]"
 )
else()
 target_compile_options(DEMND PUBLIC -g -O3 -fopenmp -Wall -Wextra)
 target_link_libraries(DEMND PUBLIC -g -O3 -fopenmp -lstdc++fs -lm -Wall -Wextra)
endif()
