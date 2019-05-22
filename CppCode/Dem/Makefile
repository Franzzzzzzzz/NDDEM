HOST=$(shell hostname)
  
ifeq "$(HOST)" "guillard"
CC=/opt/local/bin/g++-mp-6 # FG
endif
#CC=g++-8 # BM
#CC=g++

# Basic compilation

# Compilation incluant le support matlab


SOURCES=DEMND.cpp Parameters.cpp Tools.cpp Contacts.cpp ContactList.cpp Xml.cpp Multiproc.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=DEMND
LDFLAGS=-L/usr/local/lib -L/usr/lib -Wall
LIBS=-lm
FLTO=-flto
FLTO=

all: CFLAGS+=-g -O3 -I/usr/local/include -I/opt/local/include -Wall -fopenmp -lstdc++fs
all: $(SOURCES) $(EXECUTABLE)
all: LIBS+= -fopenmp -lstdc++fs #-lnetcdf
all:
	@$(CC) -o xml2vtk xml2vtk.cpp Xml.cpp -lstdc++fs
#info:#
#	@echo 'Compilation without Matlab or Boost or Tiff support. Use "make advanced" to activate them.'

texturing:
	@$(CC) -g -Wall -lstdc++fs -O3 -I/usr/local/include -I/opt/local/include -o Texturing Texturing.cpp Tools.cpp TinyPngOut.cpp

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(FLTO) $(OBJECTS) -o $@ $(LIBS)

.cpp.o:
	$(CC) $(CFLAGS) $(FLTO) $< -c -o $@

clean :
	rm *.o
