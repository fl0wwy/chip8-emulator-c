#include <stdio.h>
#include "chip8.h"

int chip8_load_rom(Chip8 *chip8, const char *filepath) {
    const int ROM_CAPACITY = CHIP8_MEMORY_SIZE - PC_INIT;
    FILE *fp;

    fp = fopen(filepath, "rb");

    if (fp == NULL) {
        perror("Error: Could not open ROM file");
        return 1;
    }

    if (fseek(fp, 0L, SEEK_END) != 0) {
        perror("fseek failed");
        fclose(fp);
        return 1;
    }

    long int bytes = ftell(fp);

    if (bytes == -1L) {
        fclose(fp);
        perror("Error determining file size");
        return 1;
    }

    if (bytes == 0) {
        fprintf(stderr, "Error: ROM file is empty.\n");
        fclose(fp);
        return 1;
    }

    if (bytes > ROM_CAPACITY) {
        fclose(fp);
        fprintf(stderr, "Error: ROM exceeds available memory.\n");
        return 1;
    }

    if (fseek(fp, 0L, SEEK_SET) != 0) {
        perror("fseek failed");
        fclose(fp);
        return 1;
    }

    int byte, i = 0;
    while ((byte = fgetc(fp)) != EOF) {
        if (i >= ROM_CAPACITY) {
            fprintf(stderr, "Error: ROM exceeds available memory.\n");
            fclose(fp);
            return 1;
        }

        chip8->memory[PC_INIT + i] = (uint8_t)byte;
        
        i++;
    }

    if (ferror(fp)) {
        fclose(fp);
        fprintf(stderr, "Stream Error: A file I/O error was detected!\n");
        return 1;
    }    

    fclose(fp);
    
    printf("ROM loaded successfully.\n");
    return 0;
}