# Pokemon Mod Suite

This suite provides tools for inspecting, modifying, and analysing data
files used in Pokémon games or related ROM-based assets. It includes
interactive SDL-based utilities, checksum helpers, and Python scripts
that automate data extraction, transformation, and patch generation.

------------------------------------------------------------------------

## Installation

### Build (for UNIX)

``` sh
make
```

Both tools require `SDL2` and `SDL2_ttf` to be installed on your system in order to compile (edit Makefile as necessary).

### Install

Install the binaries to `/usr/local/bin`:

``` sh
sudo make install
```

Install to a custom prefix:

``` sh
make install PREFIX=/opt/mytools
```

Install into a packaging root:

``` sh
make install DESTDIR=/tmp/pkgroot
```

### Uninstall

``` sh
sudo make uninstall
```

------------------------------------------------------------------------

## Usage

### Hex Editor

Launch the hex editor:

``` sh
hex_editor <file>
```

### Checksum Calculator

Compute checksum for a file:

``` sh
checksum <file>
```

You can use the `-h` flag with both programs to bring up an extensive usage message.

------------------------------------------------------------------------

## Python Scripts

Two Python utilities are included in the `src/scripts/` directory. These
scripts translate between byte arrays and specific game encodings if you 
just need some text converted quickly.

These acripts will be added to soon.
