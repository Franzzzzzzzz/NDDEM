CC=/opt/local/bin/g++-mp-6

# Basic compilation
#CFLAGS=-c -O3 -I./Headers -I/opt/local/include
#LDFLAGS=-L./newmat10
#LDFLAGS=-L/opt/local/lib
#LIBS=-lm -lpthread

# Compilation incluant le support matlab
#CFLAGS=-c -O3 -m32 -DMATLAB -I./Headers -I/opt/local/include -I/home/franz/Logiciels/matlab2009/extern/include
# -DUSETIFF and -ltiff to use tiff
#LDFLAGS=-L/opt/local/lib -L/home/franz/Logiciels/matlab2009/bin/glnx86


SOURCES=DEMND.cpp Parameters.cpp Tools.cpp Contacts.cpp ContactList.cpp Xml.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=DEMND
LDFLAGS=-L/opt/local/lib
LIBS=-lm
FLTO=-flto
FLTO=

all: CFLAGS+=-g -O3 -DNETCDF -I/opt/local/include
all: $(SOURCES) $(EXECUTABLE)
all: LIBS+=-lnetcdf
#info:#
#	@echo 'Compilation without Matlab or Boost or Tiff support. Use "make advanced" to activate them.'

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(FLTO) $(OBJECTS) -o $@ $(LIBS)

.cpp.o:
	$(CC) $(CFLAGS) $(FLTO) $< -c -o $@

clean :
	rm *.o
