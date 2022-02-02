# chip8
Very basic chip8 emulator

## Usage
```
chip8 /path/to/rom
```

## Compilation on m1
```
# First download raylib via homebrew
brew install raylib

# Create build directory and change to it
mkdir build && cd build

# Create makefile using cmake
cmake -DCMAKE_APPLE_SILICON_PROCESSOR=arm64 ..

# Build the emulator using the Makefile
make

```

#### Info
This emulator does not provide any rom files.
