#ifndef CHIP8_H
#define CHIP8_H

#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <SDL3/SDL.h>

#define CHIP8_MEMORY_SIZE 4096
#define PC_INIT 0x200
#define SEED time(NULL)
#define CHIP8_NO_KEY 16


typedef struct {    
    // CHIP-8 memory and general-purpose registers.
    uint8_t memory[CHIP8_MEMORY_SIZE];
    uint8_t V[16];
    
    uint16_t I;
    uint16_t pc;

    uint8_t st;
    uint8_t dt;

    uint8_t sp;
    uint16_t stack[16];

    // One byte per pixel in the 64x32 display.
    uint8_t gfx[64 * 32];

    uint8_t key[16];
    uint8_t held_key; // CHIP8_NO_KEY when no key is being awaited.
} Chip8;

void chip8_init(Chip8 *chip8);
int chip8_load_rom(Chip8 *chip8, const char *filepath);
int chip8_fetch(Chip8 *chip8, uint16_t *opcode);
void chip8_execute(Chip8 *chip8, uint16_t opcode);
int initialize_SDL(int width, int height);
bool process_input(Chip8 *chip8);
void draw_pixels(uint8_t gfx[64 * 32]);
void destroy_SDL();
void audio_streaming(float *samples, int count);
int generate_square_waveform(float *samples, int count, int position);
void set_audio_flag(bool condition);
void update_audio();

#endif