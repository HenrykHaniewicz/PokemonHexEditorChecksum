CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -I.
LDFLAGS = -lSDL3 -lSDL3_ttf

# Directories
OBJDIR = obj
SRCDIR = src

# Installation directories
PREFIX ?= /usr/local
BINDIR = $(PREFIX)/bin

# Source files
COMMON_SRC = $(SRCDIR)/common/sdl_app_base.cpp
HEX_EDITOR_SRC = $(SRCDIR)/hex_editor/hex_editor.cpp
CHECKSUM_SRC = $(SRCDIR)/checksum/checksum_calc.cpp
MIRAGE_ISLAND_SRC = $(SRCDIR)/mirage_island/mirage_island.cpp
HEX_EDITOR_MAIN = $(SRCDIR)/hex_editor_main.cpp
CHECKSUM_MAIN = $(SRCDIR)/checksum_main.cpp
MIRAGE_ISLAND_MAIN = $(SRCDIR)/mirage_island_main.cpp

# Object files
COMMON_OBJ = $(OBJDIR)/sdl_app_base.o
HEX_EDITOR_OBJ = $(OBJDIR)/hex_editor.o
CHECKSUM_OBJ = $(OBJDIR)/checksum_calc.o
MIRAGE_ISLAND_OBJ = $(OBJDIR)/mirage_island.o
HEX_EDITOR_MAIN_OBJ = $(OBJDIR)/hex_editor_main.o
CHECKSUM_MAIN_OBJ = $(OBJDIR)/checksum_main.o
MIRAGE_ISLAND_MAIN_OBJ = $(OBJDIR)/mirage_island_main.o

# Executables
HEX_EDITOR_BIN = hex_editor
CHECKSUM_BIN = checksum
MIRAGE_ISLAND_BIN = mirageisland

# Default target
all: $(HEX_EDITOR_BIN) $(CHECKSUM_BIN) $(MIRAGE_ISLAND_BIN)

# Link hex_editor
$(HEX_EDITOR_BIN): $(COMMON_OBJ) $(HEX_EDITOR_OBJ) $(HEX_EDITOR_MAIN_OBJ)
	$(CXX) -o $@ $^ $(LDFLAGS)

# Link checksum
$(CHECKSUM_BIN): $(COMMON_OBJ) $(CHECKSUM_OBJ) $(CHECKSUM_MAIN_OBJ)
	$(CXX) -o $@ $^ $(LDFLAGS)

# Link mirageisland
$(MIRAGE_ISLAND_BIN): $(COMMON_OBJ) $(MIRAGE_ISLAND_OBJ) $(MIRAGE_ISLAND_MAIN_OBJ)
	$(CXX) -o $@ $^ $(LDFLAGS)

# Create obj directory
$(OBJDIR):
	mkdir -p $(OBJDIR)

# Generic rule for compiling common sources
$(COMMON_OBJ): $(COMMON_SRC) | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# Generic rule for compiling hex_editor sources
$(HEX_EDITOR_OBJ): $(HEX_EDITOR_SRC) | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# Generic rule for compiling checksum sources
$(CHECKSUM_OBJ): $(CHECKSUM_SRC) | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# Generic rule for compiling mirage island sources
$(MIRAGE_ISLAND_OBJ): $(MIRAGE_ISLAND_SRC) | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# Compile hex_editor main
$(HEX_EDITOR_MAIN_OBJ): $(HEX_EDITOR_MAIN) | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# Compile mirageisland main
$(MIRAGE_ISLAND_MAIN_OBJ): $(MIRAGE_ISLAND_MAIN) | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# Compile checksum main
$(CHECKSUM_MAIN_OBJ): $(CHECKSUM_MAIN) | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# Clean build artifacts
clean:
	rm -rf $(OBJDIR)
	rm -f $(HEX_EDITOR_BIN) $(CHECKSUM_BIN) $(MIRAGE_ISLAND_BIN)

# Rebuild everything
rebuild: clean all

install: $(HEX_EDITOR_BIN) $(CHECKSUM_BIN) $(MIRAGE_ISLAND_BIN)
	mkdir -p $(DESTDIR)$(BINDIR)
	cp $(HEX_EDITOR_BIN) $(DESTDIR)$(BINDIR)/
	cp $(CHECKSUM_BIN) $(DESTDIR)$(BINDIR)/
	cp $(MIRAGE_ISLAND_BIN) $(DESTDIR)$(BINDIR)/

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/$(HEX_EDITOR_BIN)
	rm -f $(DESTDIR)$(BINDIR)/$(CHECKSUM_BIN)
	rm -f $(DESTDIR)$(BINDIR)/$(MIRAGE_ISLAND_BIN)

.PHONY: all clean rebuild