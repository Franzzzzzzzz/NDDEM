link_directories(${CMAKE_BINARY_DIR}/NrrdIO-1.11.0-src)
#add_executable(CoarseGraining IONDDEM.cpp Coarsing.cpp ../Dem/Xml.cpp zlib.cpp gzip.cpp)
add_executable(CoarseGraining Main.cpp Coarsing.cpp Reader-Liggghts.cpp Reader-Mercury.cpp Reader-Yade.cpp ../Dem/Xml.cpp zlib.cpp gzip.cpp pugixml.cpp)
set_property(TARGET CoarseGraining PROPERTY CXX_STANDARD 17)
#add_executable(CoarseGraining_Liggghts EXCLUDE_FROM_ALL IOLiggghts.cpp Coarsing.cpp zlib.cpp gzip.cpp)
#set_property(TARGET CoarseGraining_Liggghts PROPERTY CXX_STANDARD 17)

if(EMSCRIPTEN)
 target_compile_options(CoarseGraining PUBLIC -O3 ${Matlab_Macro} -Wall -DUSEBINDINGS)
 target_link_libraries(CoarseGraining PUBLIC -O3 -lz -lm -Wall -DUSEBINDINGS)
 set(MODULARIZE "1")
 set(EXPORT_NAME "CoarseGraining")
 set_target_properties(
    CoarseGraining
    PROPERTIES
    LINK_FLAGS
    "-O3 -lnodefs.js -lworkerfs.js --bind -s ALLOW_MEMORY_GROWTH=1 -s MODULARIZE=1 -s EXPORT_NAME='CoarseGraining' -s EXPORTED_RUNTIME_METHODS=[\"FS\",\"WORKERFS\"]"
 )
else()
 target_compile_options(CoarseGraining PUBLIC -Wall -g -DNRRDIO ${Matlab_Macro})
 target_link_libraries(CoarseGraining PUBLIC -Wall -g -O3 -lz -lm -lNrrdIO -lstdc++fs)
# target_compile_options(CoarseGraining_Liggghts PUBLIC -g -O3 -DNRRDIO ${Matlab_Macro} -Wall)
# target_link_libraries(CoarseGraining_Liggghts PUBLIC -g -O3 -lz -lm -lNrrdIO -lstdc++fs -Wall)
endif()




