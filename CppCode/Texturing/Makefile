HOST=$(shell hostname)

ifeq "$(HOST)" "guillard"
CC=/opt/local/bin/g++-mp-6 # FG
else ifeq "$(HOST)" "franz-pavilion"
CC=g++
endif
#CC=g++-8 # BM
#CC=g++

# Basic compilation

# Compilation incluant le support matlab


SOURCES=Texturing.cpp ../Dem/Tools.cpp TinyPngOut.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=Texturing
LDFLAGS=-L/usr/local/lib -L/usr/lib -Wall -L../CoarseGraining/NrrdIO-1.11.0-src/build
LIBS=-lm -lpthread
FLTO=-flto
FLTO=

all: CFLAGS+=-g -O3 -I/usr/local/include -I/opt/local/include -Wall -fopenmp -lstdc++fs #-DNRRDIO
all: $(SOURCES) $(EXECUTABLE)
all: LIBS+= -fopenmp -lstdc++fs -lNrrdIO #-lnetcdf
all:

#info:#
#	@echo 'Compilation without Matlab or Boost or Tiff support. Use "make advanced" to activate them.'

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(FLTO) $(OBJECTS) -o $@ $(LIBS)

.cpp.o:
	$(CC) $(CFLAGS) $(FLTO) $< -c -o $@

clean :
	rm *.o
