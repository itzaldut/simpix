# ROOT flags
ROOTCFLAGS := $(shell root-config --cflags)
ROOTGLIBS  := $(shell root-config --glibs)
ROOTLIBS   := $(shell root-config --libs)

# Compiler
CXX     := g++ -Wall -O3
CXXFLAGS := $(ROOTCFLAGS)

# Additional required libraries:
#   -lASImage for TASImage
#   and GUI libs for TApplication, TCanvas, etc.
EXTRALIBS := -lASImage

# Targets
TARGETS := simpix simpix_start

all: $(TARGETS)

simpix: simpix.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^ $(ROOTGLIBS) $(ROOTLIBS) $(EXTRALIBS)

simpix_start: simpix_start.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^ $(ROOTGLIBS) $(ROOTLIBS) $(EXTRALIBS)

clean:
	rm -f $(TARGETS) *.o out.png collage.png

