# CHIP-8 Emulator

A CHIP-8 emulator written in C using SDL3 for graphics, keyboard input, and audio. Implements the standard 64×32 display, 16-key keypad, 60 Hz delay and sound timers, and a square-wave beep.

## Build

Requires a C17 compiler, GNU Make, SDL3, and `pkg-config`. Developed on Windows with MSYS2 UCRT64.

```sh
mingw32-make
```

## Run

```sh
./chip8.exe path/to/rom.ch8
```

The keypad uses the following physical keyboard layout:

```text
Keyboard       CHIP-8
1 2 3 4        1 2 3 C
Q W E R        4 5 6 D
A S D F        7 8 9 E
Z X C V        A 0 B F
```

## Tests

Run the C unit tests with:

```sh
mingw32-make test
```

Compatibility was also checked using the [Timendus CHIP-8 Test Suite](https://github.com/Timendus/chip8-test-suite), alongside running CHIP-8 ROMs.

## References

The implementation follows [Cowgod's CHIP-8 Technical Reference](http://devernay.free.fr/hacks/chip8/C8TECH10.HTM). The [Timendus CHIP-8 Test Suite](https://github.com/Timendus/chip8-test-suite) was used for testing.
