# Edit according to your needs
SHELL=/bin/bash

PROGRAM = lazar 
FEAT_GEN = linfrag #rex smarts-features
#TOOLS = chisq-filter pcprop
INSTALLDIR = /usr/local/bin

OBJ = feature.o lazmol.o io.o rutils.o
HEADERS = lazmolvect.h feature.h lazmol.h io.h feature-generation.h rutils.h 

CC            = g++
INCLUDE       = -I/usr/local/include/openbabel-2.0/ -I/usr/local/lib/R/include/
#INCLUDE       = -I/usr/local/include/openbabel-2.0/ -I/usr/local/lib64/R/include/
CXXFLAGS      = -O3 $(INCLUDE) -Wall -fPIC
LIBS	        = -lm -ldl -lopenbabel -lgslcblas -lgsl -lRblas -lRlapack -lR 
LDFLAGS       = -L/usr/local/lib -L/usr/local/lib/R/lib
#LDFLAGS       = -L/usr/local/lib -L/usr/local/lib64/R/lib
SWIG          = swig
SWIGFLAGS     = -c++ -ruby
RUBY_INC      = -I/usr/local/lib/ruby/1.8/i686-linux/
#RUBY_INC      = -I/opt/ruby-enterprise-1.8.6-20090520/lib/ruby/1.8/x86_64-linux

%.cxx: %.i
	$(SWIG) $(SWIGFLAGS) -o $@ $^
lazar_wrap.o: lazar_wrap.cxx
	$(CC) -fPIC $(RUBY_INC) $(INCLUDE) -c -o $@ $^
lazar.so: lazar_wrap.o $(OBJ)
	$(CC) -shared $(CXXFLAGS) $(LIBS) $(LDFLAGS) $^ /usr/local/lib/libopenbabel.so /usr/lib/libgsl.so -o $@

.PHONY:
all: $(PROGRAM) $(FEAT_GEN) lazar.so #$(TOOLS)

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
	-rm -rf *.o $(PROGRAM) $(TOOLS) $(FEAT_GEN) lazar.so
