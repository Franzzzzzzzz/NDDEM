HOST=$(shell hostname)

ifeq "$(HOST)" "guillard"
CC=/opt/local/bin/g++-mp-9 # FG
CFLAGMAT=-I/Applications/MATLAB_R2017a.app/extern/include -DMATLAB
LIBFLAGMAT=-L/Applications/MATLAB_R2017a.app/bin/maci64
else ifeq "$(HOST)" "franz-pavilion"
CC=g++
CFLAGMAT=-I/home/franz/Matlab2017a/extern/include -DMATLAB
LIBFLAGMAT=-L/home/franz/Matlab2017a/bin/glnxa64 
else
CC=g++
endif


# Basic compilation
#CFLAGS=-c -O3 -I./Headers -I/opt/local/include
#LDFLAGS=-L./newmat10
#LDFLAGS=-L/opt/local/lib
#LIBS=-lm -lpthread

# Compilation incluant le support matlab
#CFLAGS=-c -O3 -m32 -DMATLAB -I./Headers -I/opt/local/include -I/home/franz/Logiciels/matlab2009/extern/include
# -DUSETIFF and -ltiff to use tiff
#LDFLAGS=-L/opt/local/lib -L/home/franz/Logiciels/matlab2009/bin/glnx86


SOURCES=IONDDEM.cpp Coarsing.cpp ../Dem/Xml.cpp zlib.cpp gzip.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=IO
CFLAGS=-g -I/opt/local/include -DNETCDF -DNRRDIO $(CFLAGMAT)
LDFLAGS=-g -L/opt/local/lib -L./NrrdIO-1.11.0-src/build $(LIBFLAGMAT)
LIBS=-lz -lm -lnetcdf -lNrrdIO -lmat -lmx

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@ $(LIBS)

.cpp.o:
	$(CC) $(CFLAGS) $< -c -o $@

clean :
	rm *.o
