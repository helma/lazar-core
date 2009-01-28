# Edit according to your needs
SHELL=/bin/bash

PROGRAM = lazar 
FEAT_GEN = rex linfrag smarts-features
TOOLS = chisq-filter #pcprop
INSTALLDIR = /usr/local/bin

OBJ = feature.o lazmol.o io.o rutils.o
HEADERS = lazmolvect.h feature.h lazmol.h io.h ServerSocket.h Socket.h feature-generation.h rutils.h
SERVER_OBJ = ServerSocket.o Socket.o
OBJ += $(SERVER_OBJ)

CC            = g++
#CXXFLAGS      = -g -O2 -I/usr/include/openbabel-2.0/ -I../R-2.8.0/include/ -I../gsl-1.9/bin/include/ -Wall
CXXFLAGS      = -g -O2 -I/usr/local/include/openbabel-2.0/ -I/usr/local/lib/R/include -Wall
LIBS	      = -lm -ldl -lopenbabel -lgslcblas -lgsl -lRblas -lRlapack -lR
LDFLAGS       = -L/usr/local/lib/R/lib
#LDFLAGS       = -L../gsl-1.9/bin/lib -L../R-2.8.0/lib -L../R-2.8.0/bin/lib64/R/modules/
#RPATH         = -Wl,-rpath=../R-2.8.0/lib

.PHONY:
all: $(PROGRAM) $(FEAT_GEN) $(TOOLS)

.PHONY:
doc: Doxyfile
	doxygen Doxyfile

lazar: $(OBJ)  lazar.o 
	$(CC) $(CXXFLAGS) $(INCLUDE) $(LIBS) $(LDFLAGS) $(RPATH) -o lazar $(OBJ)  lazar.o 

linfrag: $(OBJ) linfrag.o
	$(CC) $(CXXFLAGS) $(INCLUDE) $(LIBS) $(LDFLAGS) $(RPATH) -o linfrag $(OBJ) linfrag.o

pcprop: $(OBJ) pcprop.o
	$(CC) $(CXXFLAGS) $(INCLUDE) $(LIBS) $(LDFLAGS) $(RPATH) -o pcprop $(OBJ) pcprop.o

rex: $(OBJ) rex.o
	$(CC) $(CXXFLAGS) $(INCLUDE) $(LIBS) $(LDFLAGS) $(RPATH) -o rex $(OBJ) rex.o

chisq-filter: $(OBJ)  chisq-filter.o 
	$(CC) $(CXXFLAGS) $(INCLUDE) $(LIBS) $(LDFLAGS) $(RPATH) -o chisq-filter $(OBJ)  chisq-filter.o 

smarts-features: $(OBJ)  smarts-features.o 
	$(CC) $(CXXFLAGS) $(INCLUDE) $(LIBS) $(LDFLAGS) $(RPATH) -o smarts-features $(OBJ)  smarts-features.o 

testset: $(OBJ)  testset.o 
	$(CC) $(CXXFLAGS) $(INCLUDE) $(LIBS) $(LDFLAGS) $(RPATH) -o testset $(OBJ)  testset.o 

chisq-filter.o: $(HEADERS) activity-db.h feature-db.h 

rex.o: $(HEADERS) feature-generation.h

linfrag.o: $(HEADERS) feature-generation.h

pcprop.o: $(HEADERS)

smarts-features.o: $(HEADERS) feature-generation.h

lazar.o: $(HEADERS) predictor.h model.h activity-db.h feature-db.h feature-generation.h

lazmol.o: lazmol.h

feature.o: feature.h 

io.o: io.cpp io.h $(SERVER_OBJ)

rutils.o: rutils.h

testset.o: feature-generation.h

.PHONY:
clean:
	-rm -rf *.o $(PROGRAM) $(TOOLS) $(FEAT_GEN)
