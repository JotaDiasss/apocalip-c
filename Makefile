

CC = gcc
PKG_CFLAGS := $(shell pkg-config --cflags sdl2 SDL2_ttf 2>/dev/null)
PKG_LIBS := $(shell pkg-config --libs sdl2 SDL2_ttf 2>/dev/null)

ifeq ($(PKG_CFLAGS),)
CFLAGS = -O2 -Wall
else
CFLAGS = -O2 -Wall $(PKG_CFLAGS)
endif

ifeq ($(PKG_LIBS),)
# Fallback for MinGW/Windows if pkg-config isn't available
LDFLAGS = -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf
else
LDFLAGS = $(PKG_LIBS)
endif

TARGET = apocalip

all: $(TARGET)

$(TARGET): main.c
	$(CC) $(CFLAGS) -o $(TARGET) main.c $(LDFLAGS)

clean:
	rm -f $(TARGET)
