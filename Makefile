CXX := g++
 
OUTPUTDIR := out
OBJDIR := objects
SOURCEDIR := src
OBJSOURCES := $(addprefix $(OBJDIR)/,LZ77.o)

SOURCES = $(shell find $(SOURCEDIR) -type f -name *.cpp)
OBJECTS = $(patsubst $(SOURCEDIR)/%.cpp,$(OBJDIR)/%.o,$(SOURCES))
 
all: $(OUTPUTDIR)/main simpleclean
 
$(OUTPUTDIR)/main: $(OBJECTS) | $(OBJDIR)
	$(CXX) -o lz $^
 
$(OBJDIR)/%.o: $(SOURCEDIR)/%.cpp | $(OBJDIR)
	$(CXX) -c -o $@ $<
 
$(OUTPUTDIR):
	mkdir -p $(OUTPUTDIR)
 
$(OBJDIR):
	mkdir -p $(OBJDIR)
 
.PHONY: clean simpleclean
 
clean: simpleclean
	rm -rf $(OUTPUTDIR)
 
simpleclean:
	rm -rf $(OBJDIR)
