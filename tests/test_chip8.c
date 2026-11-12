#include <assert.h>
#include <stdio.h>
#include "chip8.h"

int main(void)
{
    Chip8 chip8;

    chip8_init(&chip8);

    assert(chip8.pc == 0x200);
    assert(chip8.memory[0x050] == 0xF0);
    assert(chip8.memory[0x09F] == 0x80);
    assert(chip8.V[0] == 0);

    printf("[PASS] CHIP-8 initialization\n");

    return 0;
}