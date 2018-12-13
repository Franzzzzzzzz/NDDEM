CC=g++

# Basic compilation
#CFLAGS=-c -O3 -I./Headers -I/opt/local/include
#LDFLAGS=-L./newmat10 
#LDFLAGS=-L/opt/local/lib
#LIBS=-lm -lpthread 

# Compilation incluant le support matlab
#CFLAGS=-c -O3 -m32 -DMATLAB -I./Headers -I/opt/local/include -I/home/franz/Logiciels/matlab2009/extern/include
# -DUSETIFF and -ltiff to use tiff
#LDFLAGS=-L/opt/local/lib -L/home/franz/Logiciels/matlab2009/bin/glnx86


SOURCES=IO.cpp Coarsing.cpp zlib.cpp gzip.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=IO
CFLAGS=-I/opt/local/include -DNETCDF
LDFLAGS=-L/opt/local/lib
LIBS=-lz -lm -lnetcdf

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@ $(LIBS)

.cpp.o:
	$(CC) $(CFLAGS) $< -c -o $@

clean : 
	rm *.o
