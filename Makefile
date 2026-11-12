CC = gcc
CFLAGS = -std=c17 -Wall -Wextra -Wpedantic -g -Iinclude

SDL_CFLAGS := $(shell pkg-config --cflags sdl3)
SDL_LIBS   := $(shell pkg-config --libs sdl3)


SOURCES = src/main.c src/chip8.c src/cpu.c src/rom.c src/sdl.c
TARGET = chip8.exe

.PHONY: all clean test

all: $(TARGET)

$(TARGET): $(SOURCES) include/chip8.h
	$(CC) $(CFLAGS) $(SDL_CFLAGS) $(SOURCES) -o $(TARGET) $(SDL_LIBS)


test: tests/test_chip8.exe tests/test_rom.exe tests/test_cpu.exe
	./tests/test_chip8.exe
	./tests/test_rom.exe
	./tests/test_cpu.exe

tests/test_chip8.exe: tests/test_chip8.c src/chip8.c include/chip8.h
	$(CC) $(CFLAGS) tests/test_chip8.c src/chip8.c -o tests/test_chip8.exe

tests/test_rom.exe: tests/test_rom.c src/rom.c src/chip8.c include/chip8.h
	$(CC) $(CFLAGS) tests/test_rom.c src/rom.c src/chip8.c -o tests/test_rom.exe	

tests/test_cpu.exe: tests/test_cpu.c src/cpu.c src/chip8.c include/chip8.h
	$(CC) $(CFLAGS) tests/test_cpu.c src/cpu.c src/chip8.c -o tests/test_cpu.exe	


clean:
	rm -f $(TARGET) tests/*.exe

