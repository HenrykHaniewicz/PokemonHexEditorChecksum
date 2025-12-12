CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -I. -MMD -MP
LDFLAGS = -lSDL3 -lSDL3_ttf

# Directories
OBJDIR = obj
SRCDIR = src

# Installation directories
PREFIX ?= /usr/local
BINDIR = $(PREFIX)/bin

# Find all source files automatically
SRCS := $(shell find $(SRCDIR) -name '*.cpp')

# Generate object file names (flatten to obj directory)
OBJS := $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(subst /,_,$(SRCS:$(SRCDIR)/%=%)))

# Generate dependency file names
DEPS := $(OBJS:.o=.d)

# Executables and their required objects
HEX_EDITOR_BIN = hex_editor
CHECKSUM_BIN = checksum
MIRAGE_ISLAND_BIN = mirageisland
POKEMON_BAG_BIN = pokemon_bag
POKEMON_PARTY_BIN = pokemon_party

BINS = $(HEX_EDITOR_BIN) $(CHECKSUM_BIN) $(MIRAGE_ISLAND_BIN) $(POKEMON_BAG_BIN) $(POKEMON_PARTY_BIN)

# Common objects used by multiple targets
COMMON_OBJS = $(OBJDIR)/common_sdl_app_base.o
GEN3_OBJS = $(OBJDIR)/common_generation3_utils.o

# Object lists for each executable
HEX_EDITOR_OBJS = $(COMMON_OBJS) \
                  $(OBJDIR)/hex_editor_hex_editor.o \
                  $(OBJDIR)/hex_editor_main.o

CHECKSUM_OBJS = $(COMMON_OBJS) $(GEN3_OBJS) \
                $(OBJDIR)/checksum_checksum_calc.o \
                $(OBJDIR)/checksum_main.o

MIRAGE_ISLAND_OBJS = $(COMMON_OBJS) $(GEN3_OBJS) \
                     $(OBJDIR)/mirage_island_mirage_island.o \
                     $(OBJDIR)/mirage_island_main.o

POKEMON_BAG_OBJS = $(COMMON_OBJS) $(GEN3_OBJS) \
                   $(OBJDIR)/pokemon_bag_pokemon_bag.o \
                   $(OBJDIR)/pokemon_bag_main.o

POKEMON_PARTY_OBJS = $(COMMON_OBJS) $(GEN3_OBJS) \
                   $(OBJDIR)/pokemon_party_pokemon_party.o \
                   $(OBJDIR)/pokemon_party_main.o

# Default target
all: $(BINS)

# Create obj directory
$(OBJDIR):
	mkdir -p $(OBJDIR)

# Generic compilation rule - converts path separators to underscores
# The VPATH mechanism doesn't work well with flattened output
define make_obj_rule
$(OBJDIR)/$(subst /,_,$(1:$(SRCDIR)/%.cpp=%.o)): $(1) | $(OBJDIR)
	$$(CXX) $$(CXXFLAGS) -c -o $$@ $$<
endef

# Generate a rule for each source file
$(foreach src,$(SRCS),$(eval $(call make_obj_rule,$(src))))

# Link executables
$(HEX_EDITOR_BIN): $(HEX_EDITOR_OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS)

$(CHECKSUM_BIN): $(CHECKSUM_OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS)

$(MIRAGE_ISLAND_BIN): $(MIRAGE_ISLAND_OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS)

$(POKEMON_BAG_BIN): $(POKEMON_BAG_OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS)

$(POKEMON_PARTY_BIN): $(POKEMON_PARTY_OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS)

# Include dependency files (if they exist)
-include $(DEPS)

# Clean build artifacts
clean:
	rm -rf $(OBJDIR)
	rm -f $(BINS)

# Rebuild everything
rebuild: clean all

# Installation
install: $(BINS)
	mkdir -p $(DESTDIR)$(BINDIR)
	cp $(BINS) $(DESTDIR)$(BINDIR)/

uninstall:
	$(foreach bin,$(BINS),rm -f $(DESTDIR)$(BINDIR)/$(bin);)

.PHONY: all clean rebuild install uninstall