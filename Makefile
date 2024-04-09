# Set Directories
SRCDIR   := $(CURDIR)/src
INCDIR   := $(CURDIR)/include
BUILDDIR := $(CURDIR)/build
#DATADIR  := $(CURDIR)/data

# Compiler
CC      := g++
# Compiler flags
CFLAGS  := -std=c++17 -Wall -Wextra -I$(INCDIR)
# Linker flags
LDFLAGS :=

# Files
SOURCES    := $(wildcard $(SRCDIR)/*.cpp)
OBJECTS    := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.cpp=.o))
EXECUTABLE := runfile
#DATA       := $(wildcard $(DATADIR)/*)

# Build directory creation
$(shell mkdir -p $(BUILDDIR))

# Build executable
$(BUILDDIR)/$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

# Compile source files
$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp
	$(CC) $(CFLAGS) -c -o $@ $<

# Phony targets
.PHONY: clean #data

clean:
	rm -f $(BUILDDIR)/*.o $(BUILDDIR)/$(EXECUTABLE)

#data:
#	cp -r $(DATADIR) $(BUILDDIR)/
