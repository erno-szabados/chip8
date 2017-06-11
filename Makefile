TARGET = chip8e
LIBS   = -lm
CC     = cc
# POSIX 2008 for setenv in stdlib:
CFLAGS = -g -Wall -std=c99 -D_XOPEN_SOURCE=700
LDFLAGS=
LIBS   =

default: $(TARGET)
all: default

SOURCES = stack.c sprites.c chip8.c main.c 
OBJECTS = stack.o sprites.o chip8.o main.o 

$(TARGET): $(OBJECTS)
	$(CC) -o $(TARGET) $(OBJECTS) $(LDFLAGS) $(LIBS)
	
.PHONY: default all clean

clean:
	-rm -f $(OBJECTS) $(TARGET)


