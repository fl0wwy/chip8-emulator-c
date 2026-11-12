#include <stdio.h>
#include "chip8.h"

int main(int argc, char *argv[]) {
    Chip8 chip8;
    uint16_t opcode;

    bool running = true;

    const double CPU_INTERVAL = 1.0 / 700.0;
    const double TIMER_INTERVAL = 1.0 / 60.0;
    const double RENDER_INTERVAL = 1.0 / 60.0;

    double cpu_accumulator = 0.0;
    double timer_accumulator = 0.0;
    double render_accumulator = 0.0;

    uint64_t current_time;
    double elapsed;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <rom_path>\n", argv[0]);
        return 1;
    }

    chip8_init(&chip8);

    if (chip8_load_rom(&chip8, argv[1]) != 0) {
        return 1;
    }

    if (initialize_SDL(800, 600) != 0) {
        destroy_SDL();
        return 1;
    }

    srand(SEED);
    uint64_t previous_time = SDL_GetTicksNS();
    
    while (running) {
        current_time = SDL_GetTicksNS();

        elapsed = (double)(current_time - previous_time) / SDL_NS_PER_SECOND;
        previous_time = current_time;
        
        cpu_accumulator += elapsed;
        timer_accumulator += elapsed;
        render_accumulator += elapsed;

        running = process_input(&chip8);
        
        if (!running){
            break;
        };

        while (cpu_accumulator >= CPU_INTERVAL) {
            if (chip8_fetch(&chip8, &opcode) != 0) {
                running = false;
                break;
            }

            chip8_execute(&chip8, opcode);

            cpu_accumulator -= CPU_INTERVAL;
        }

        if (!running){
            break;
        };

        
        while (timer_accumulator >= TIMER_INTERVAL) {
            if (chip8.dt > 0){
                chip8.dt--;
            }

            if (chip8.st > 0){
                chip8.st--;
            }

            timer_accumulator -= TIMER_INTERVAL;
        }

        set_audio_flag(chip8.st > 0);
        update_audio();

        if (render_accumulator >= RENDER_INTERVAL) {
            draw_pixels(chip8.gfx);

            render_accumulator -= RENDER_INTERVAL;
        }

        SDL_Delay(1);
    }

    destroy_SDL();
    return 0;
}