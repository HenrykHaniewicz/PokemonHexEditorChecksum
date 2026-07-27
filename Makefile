CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra -I. -MMD -MP
LDFLAGS = -lSDL3 -lSDL3_ttf

# Host OS (used to decide whether to bundle a macOS .app)
UNAME_S := $(shell uname -s)

# Directories
OBJDIR = obj
SRCDIR = src

# Installation directories
PREFIX ?= /usr/local
BINDIR = $(PREFIX)/bin

# Find all source files automatically, excluding the unified launcher
SRCS := $(shell find $(SRCDIR) -name '*.cpp' ! -path '$(SRCDIR)/main.cpp')

# Generate object file names (flatten to obj directory)
OBJS := $(addprefix $(OBJDIR)/,$(patsubst %.cpp,%.o,$(subst /,_,$(SRCS:$(SRCDIR)/%=%))))

# Generate dependency file names
DEPS := $(OBJS:.o=.d)

# ---- Individual executables ----

HEX_EDITOR_BIN = hex_editor
CHECKSUM_BIN = checksum
MIRAGE_ISLAND_BIN = mirageisland
POKEMON_BAG_BIN = pokemon_bag
POKEMON_PARTY_BIN = pokemon_party
POKEMON_TRAINER_BIN = pokemon_trainer

BINS = $(HEX_EDITOR_BIN) $(CHECKSUM_BIN) $(MIRAGE_ISLAND_BIN) $(POKEMON_BAG_BIN) $(POKEMON_PARTY_BIN) $(POKEMON_TRAINER_BIN)

# ---- Unified single executable ----

SINGLE_BIN = pokemon_tools

# ---- macOS app bundle settings ----

APP_NAME       = PokemonTools
APP_DIR        = $(APP_NAME).app
APP_CONTENTS   = $(APP_DIR)/Contents
APP_MACOS      = $(APP_CONTENTS)/MacOS
APP_RESOURCES  = $(APP_CONTENTS)/Resources
APP_FRAMEWORKS = $(APP_CONTENTS)/Frameworks
APP_ICON_SRC   = icon.png
APP_ICON_ICNS  = AppIcon.icns

# Common objects used by multiple targets
COMMON_OBJS = $(OBJDIR)/common_sdl_app_base.o
GEN1_OBJS = $(OBJDIR)/common_generation1_utils.o
GEN2_OBJS = $(OBJDIR)/common_generation2_utils.o
GEN3_OBJS = $(OBJDIR)/common_generation3_utils.o

# Object lists for each individual executable
HEX_EDITOR_OBJS = $(COMMON_OBJS) \
                  $(OBJDIR)/hex_editor_hex_editor.o \
                  $(OBJDIR)/hex_editor_main.o

CHECKSUM_OBJS = $(COMMON_OBJS) $(GEN1_OBJS) $(GEN2_OBJS) $(GEN3_OBJS) \
                $(OBJDIR)/checksum_checksum_calc.o \
                $(OBJDIR)/checksum_main.o

MIRAGE_ISLAND_OBJS = $(COMMON_OBJS) $(GEN3_OBJS) \
                     $(OBJDIR)/mirage_island_mirage_island.o \
                     $(OBJDIR)/mirage_island_main.o

POKEMON_BAG_OBJS = $(COMMON_OBJS) $(GEN1_OBJS) $(GEN2_OBJS) $(GEN3_OBJS) \
                   $(OBJDIR)/pokemon_bag_pokemon_bag.o \
                   $(OBJDIR)/pokemon_bag_main.o

POKEMON_PARTY_OBJS = $(COMMON_OBJS) $(GEN1_OBJS) $(GEN2_OBJS) $(GEN3_OBJS) \
                   $(OBJDIR)/pokemon_party_pokemon_party.o \
                   $(OBJDIR)/pokemon_party_main.o

POKEMON_TRAINER_OBJS = $(COMMON_OBJS) $(GEN2_OBJS) $(GEN3_OBJS) \
                       $(OBJDIR)/pokemon_trainer_pokemon_trainer.o \
                       $(OBJDIR)/pokemon_trainer_main.o

# Objects for the unified build: every tool library + unified main
# (excludes the six individual *_main.o files)
SINGLE_OBJS = $(COMMON_OBJS) $(GEN1_OBJS) $(GEN2_OBJS) $(GEN3_OBJS) \
              $(OBJDIR)/hex_editor_hex_editor.o \
              $(OBJDIR)/checksum_checksum_calc.o \
              $(OBJDIR)/mirage_island_mirage_island.o \
              $(OBJDIR)/pokemon_bag_pokemon_bag.o \
              $(OBJDIR)/pokemon_party_pokemon_party.o \
              $(OBJDIR)/pokemon_trainer_pokemon_trainer.o \
              $(OBJDIR)/main.o

# Info.plist contents (written verbatim into the bundle). CFBundleExecutable
# points at the launcher script (named $(SINGLE_BIN)), which wraps the real
# Mach-O binary ($(SINGLE_BIN)-bin).
define INFO_PLIST_CONTENT
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
  <key>CFBundleName</key>
  <string>Pokemon Tools</string>
  <key>CFBundleDisplayName</key>
  <string>Pokemon Tools</string>
  <key>CFBundleIdentifier</key>
  <string>com.example.pokemontools</string>
  <key>CFBundleVersion</key>
  <string>1.0.0</string>
  <key>CFBundleShortVersionString</key>
  <string>1.0.0</string>
  <key>CFBundleExecutable</key>
  <string>$(SINGLE_BIN)</string>
  <key>CFBundlePackageType</key>
  <string>APPL</string>
  <key>CFBundleIconFile</key>
  <string>AppIcon</string>
  <key>NSHighResolutionCapable</key>
  <true/>
  <key>LSMinimumSystemVersion</key>
  <string>10.13</string>
</dict>
</plist>
endef
export INFO_PLIST_CONTENT

# Launcher script (the bundle's CFBundleExecutable). It verifies that the
# non-bundled dynamic dependencies of the bundled SDL3 libraries (harfbuzz,
# freetype, ...) are present; if any are missing it opens a Terminal window
# with Homebrew instructions instead of dying with a silent dyld error.
define LAUNCHER_SCRIPT
#!/bin/bash
HERE="$$(cd "$$(dirname "$$0")" && pwd)"
REAL="$$HERE/$(SINGLE_BIN)-bin"
FW="$$HERE/../Frameworks"

missing=""
if command -v otool >/dev/null 2>&1; then
    for lib in "$$FW"/libSDL3*.dylib; do
        [ -f "$$lib" ] || continue
        while read -r dep; do
            case "$$dep" in
                @*|/usr/lib/*|/System/*|"") continue ;;
            esac
            [ -f "$$dep" ] || missing="yes"
        done < <(otool -L "$$lib" | tail -n +2 | awk '{print $$1}')
    done
fi

if [ -n "$$missing" ]; then
    /usr/bin/osascript <<'APPLESCRIPT'
tell application "Terminal"
    activate
    do script "echo; echo 'Pokemon Tools could not start: required libraries are missing.'; echo 'Install them with Homebrew, then relaunch the app:'; echo; echo '    brew install harfbuzz freetype'; echo"
end tell
APPLESCRIPT
    exit 1
fi

exec "$$REAL" "$$@"
endef
export LAUNCHER_SCRIPT

# ---- Targets ----

# Default: build the six individual programs
all: $(BINS)

# Unified single-binary build
single: $(SINGLE_BIN)

# Create obj directory
$(OBJDIR):
	mkdir -p $(OBJDIR)

# Generic compilation rule - converts path separators to underscores
# The VPATH mechanism doesn't work well with flattened output
define make_obj_rule
$(OBJDIR)/$(subst /,_,$(1:$(SRCDIR)/%.cpp=%.o)): $(1) | $(OBJDIR)
	$$(CXX) $$(CXXFLAGS) -c -o $$@ $$<
endef

# Generate a rule for each source file (everything except src/main.cpp)
$(foreach src,$(SRCS),$(eval $(call make_obj_rule,$(src))))

# Explicit rule for the unified launcher main (not in SRCS)
$(OBJDIR)/main.o: $(SRCDIR)/main.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# ---- Link individual executables ----

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

$(POKEMON_TRAINER_BIN): $(POKEMON_TRAINER_OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS)

# ---- Link unified executable ----

$(SINGLE_BIN): $(SINGLE_OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS)

# ---- macOS .app bundle ----
# On Darwin: build pokemon_tools, wrap it in a .app, bundle the SDL3 dylibs
# into Contents/Frameworks and rewrite their load paths.
# Elsewhere: build pokemon_tools only and skip bundling.
app: $(SINGLE_BIN)
ifeq ($(UNAME_S),Darwin)
	@echo "==> Building macOS app bundle: $(APP_DIR)"
	@rm -rf "$(APP_DIR)"
	@mkdir -p "$(APP_MACOS)" "$(APP_RESOURCES)" "$(APP_FRAMEWORKS)"
	@# Real Mach-O binary; CFBundleExecutable is a launcher script wrapping it
	@cp "$(SINGLE_BIN)" "$(APP_MACOS)/$(SINGLE_BIN)-bin"
	@chmod u+w "$(APP_MACOS)/$(SINGLE_BIN)-bin"
	@printf '%s\n' "$$INFO_PLIST_CONTENT" > "$(APP_CONTENTS)/Info.plist"
	@printf '%s\n' "$$LAUNCHER_SCRIPT" > "$(APP_MACOS)/$(SINGLE_BIN)"
	@chmod +x "$(APP_MACOS)/$(SINGLE_BIN)"
	@echo "    Bundling SDL3 libraries into Frameworks/..."
	@EXE="$(APP_MACOS)/$(SINGLE_BIN)-bin"; \
	for lib in $$(otool -L "$$EXE" | awk 'NR>1 {print $$1}' | grep -i 'libSDL3'); do \
		base="$$(basename "$$lib")"; \
		if [ -f "$$lib" ]; then \
			cp -f "$$lib" "$(APP_FRAMEWORKS)/$$base"; \
			chmod u+w "$(APP_FRAMEWORKS)/$$base"; \
			install_name_tool -change "$$lib" "@executable_path/../Frameworks/$$base" "$$EXE"; \
			echo "      + $$base"; \
		else \
			echo "      ! not found: $$lib (skipped)"; \
		fi; \
	done
	@# Fix bundled dylib IDs and rewrite inter-library refs to @loader_path
	@for lib in "$(APP_FRAMEWORKS)"/libSDL3*.dylib; do \
		[ -f "$$lib" ] || continue; \
		base="$$(basename "$$lib")"; \
		install_name_tool -id "@executable_path/../Frameworks/$$base" "$$lib" 2>/dev/null || true; \
		otool -L "$$lib" | tail -n +2 | awk '{print $$1}' | while read -r dep; do \
			depbase="$$(basename "$$dep")"; \
			if [ -f "$(APP_FRAMEWORKS)/$$depbase" ] && [ "$$dep" != "@loader_path/$$depbase" ]; then \
				install_name_tool -change "$$dep" "@loader_path/$$depbase" "$$lib" 2>/dev/null || true; \
			fi; \
		done; \
	done
	@# Ad-hoc re-sign modified Mach-O files (required on Apple Silicon)
	@for f in "$(APP_FRAMEWORKS)"/libSDL3*.dylib "$(APP_MACOS)/$(SINGLE_BIN)-bin"; do \
		[ -f "$$f" ] && (codesign --force --sign - "$$f" >/dev/null 2>&1 || true); \
	done
	@# App icon (optional - bundle builds fine without icon.png)
	@if [ -f "$(APP_ICON_SRC)" ]; then \
		echo "    Found $(APP_ICON_SRC); generating $(APP_ICON_ICNS)..."; \
		TMP_ICON="$$(mktemp -d)"; \
		ICONSET="$$TMP_ICON/AppIcon.iconset"; \
		mkdir -p "$$ICONSET"; \
		ok=1; \
		for s in 16 32 128 256 512; do \
			s2=$$((s * 2)); \
			sips -z $$s  $$s  "$(APP_ICON_SRC)" --out "$$ICONSET/icon_$${s}x$${s}.png"    >/dev/null 2>&1 || ok=0; \
			sips -z $$s2 $$s2 "$(APP_ICON_SRC)" --out "$$ICONSET/icon_$${s}x$${s}@2x.png" >/dev/null 2>&1 || ok=0; \
		done; \
		if [ $$ok -eq 1 ] && iconutil -c icns "$$ICONSET" -o "$(APP_RESOURCES)/$(APP_ICON_ICNS)" >/dev/null 2>&1; then \
			echo "    Icon created with iconutil."; \
		elif sips -s format icns "$(APP_ICON_SRC)" --out "$(APP_RESOURCES)/$(APP_ICON_ICNS)" >/dev/null 2>&1; then \
			echo "    Icon created with sips fallback."; \
		else \
			echo "    Warning: icon conversion failed; using default icon."; \
		fi; \
		rm -rf "$$TMP_ICON"; \
	else \
		echo "    No $(APP_ICON_SRC) in project root; Resources/ left empty."; \
	fi
	@echo "==> Finished: $(APP_DIR)"
else
	@echo "==> 'make app' on non-macOS ($(UNAME_S)): built $(SINGLE_BIN), skipped .app bundle."
endif

# Include dependency files (covers both individual and unified builds)
-include $(DEPS)
-include $(OBJDIR)/main.d

# Clean build artifacts
clean:
	rm -rf $(OBJDIR)
	rm -f $(BINS) $(SINGLE_BIN)
	rm -rf "$(APP_DIR)"

# Rebuild everything
rebuild: clean all

# Installation
install: $(BINS)
	mkdir -p $(DESTDIR)$(BINDIR)
	cp $(BINS) $(DESTDIR)$(BINDIR)/

install-single: $(SINGLE_BIN)
	mkdir -p $(DESTDIR)$(BINDIR)
	cp $(SINGLE_BIN) $(DESTDIR)$(BINDIR)/

uninstall:
	$(foreach bin,$(BINS),rm -f $(DESTDIR)$(BINDIR)/$(bin);)
	rm -f $(DESTDIR)$(BINDIR)/$(SINGLE_BIN)

.PHONY: all single app clean rebuild install install-single uninstall