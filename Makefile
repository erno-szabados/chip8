TARGET     = chip8e
LIBS       = -lm
CC         = cc
SDL_CFLAGS = -I/usr/include/SDL2 -I/usr/include -D_REENTRANT -D_THREAD_SAFE
CFLAGS     = -g -Wall -std=c99 -D_XOPEN_SOURCE=700 $(SDL_CFLAGS)
LDFLAGS    =
SDL_LIBS   = -lSDL2
LIBS       = $(SDL_LIBS)

default: $(TARGET)
all: default

SOURCES = stack.c sprites.c chip8.c main.c
OBJECTS = stack.o sprites.o chip8.o main.o

$(TARGET): $(OBJECTS)
	$(CC) -o $(TARGET) $(OBJECTS) $(LDFLAGS) $(LIBS)

.PHONY: default all clean

clean:
	-rm -f $(OBJECTS) $(TARGET)


