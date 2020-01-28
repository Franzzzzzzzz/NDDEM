
link_directories(/Applications/MATLAB_R2017a.app/bin/maci64 ${CMAKE_BINARY_DIR}/NrrdIO-1.11.0-src)
add_executable(IO IONDDEM.cpp Coarsing.cpp ../Dem/Xml.cpp zlib.cpp gzip.cpp)
set_property(TARGET IO PROPERTY CXX_STANDARD 14)
target_compile_options(IO PUBLIC -g -DNRRDIO ${Matlab_Macro})
#target_include_directories(IO PUBLIC /opt/local/include)
target_link_libraries(IO PUBLIC -g -lz -lm -lNrrdIO)
