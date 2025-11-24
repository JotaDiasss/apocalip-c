#!/usr/bin/env bash
# Build script for Linux / MSYS2 (MinGW64)
set -e
echo "Building apocalip..."
if command -v pkg-config >/dev/null 2>&1; then
  CFLAGS=$(pkg-config --cflags sdl2 SDL2_ttf)
  LIBS=$(pkg-config --libs sdl2 SDL2_ttf)
  gcc main.c -o apocalip $CFLAGS $LIBS
else
  echo "pkg-config not found; trying fallback linking (may fail)"
  gcc main.c -o apocalip -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf
fi
echo "Build finished. Run ./apocalip"