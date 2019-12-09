HOST=$(shell hostname)

ifeq "$(HOST)" "guillard"
CC=/opt/local/bin/g++-mp-9 # FG
else ifeq "$(HOST)" "franz-pavilion"
CC=g++
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
CFLAGS=-g -I/opt/local/include -I/Applications/MATLAB_R2017a.app/extern/include -DNETCDF -DNRRDIO -DMATLAB
LDFLAGS=-g -L/opt/local/lib -L./NrrdIO-1.11.0-src/build -L/Applications/MATLAB_R2017a.app/bin/maci64
LIBS=-lz -lm -lnetcdf -lNrrdIO -lmat -lmx

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@ $(LIBS)

.cpp.o:
	$(CC) $(CFLAGS) $< -c -o $@

clean :
	rm *.o
