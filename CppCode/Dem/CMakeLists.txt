add_executable(DEMND DEMND.cpp Parameters.cpp Tools.cpp Contacts.cpp ContactList.cpp Xml.cpp Multiproc.cpp)
set_property(TARGET DEMND PROPERTY CXX_STANDARD 14)
target_compile_options(DEMND PUBLIC -g -O3 -fopenmp -Wall -Wextra)
target_link_libraries(DEMND PUBLIC -g -O3 -fopenmp -lstdc++fs -lm -Wall -Wextra)
