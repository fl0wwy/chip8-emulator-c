#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "chip8.h"

#define TEST_ROM_PATH "tests/test_temp.ch8"
#define ROM_CAPACITY (CHIP8_MEMORY_SIZE - PC_INIT)

/*
 * Creates a temporary ROM containing 'size' bytes.
 * Each byte follows the pattern: 0, 1, 2, ..., 255, 0, ...
 */
static void create_test_rom(size_t size)
{
    FILE *fp = fopen(TEST_ROM_PATH, "wb");
    assert(fp != NULL);

    for (size_t i = 0; i < size; i++) {
        assert(fputc((int)(i % 256), fp) != EOF);
    }

    assert(fclose(fp) == 0);
}

/* Verify that a valid ROM is loaded at the correct address. */
static void test_valid_rom(void)
{
    Chip8 chip8;
    chip8_init(&chip8);

    create_test_rom(6);

    assert(chip8_load_rom(&chip8, TEST_ROM_PATH) == 0);

    for (int i = 0; i < 6; i++) {
        assert(chip8.memory[PC_INIT + i] == (uint8_t)i);
    }

    assert(chip8.pc == PC_INIT);

    puts("[PASS] Valid ROM loading");
}

/* Verify that a nonexistent ROM is rejected. */
static void test_missing_rom(void)
{
    Chip8 chip8;
    chip8_init(&chip8);

    assert(chip8_load_rom(&chip8, "tests/nonexistent.ch8") != 0);

    puts("[PASS] Missing ROM rejection");
}

/* Verify that the maximum supported ROM size is accepted. */
static void test_maximum_rom(void)
{
    Chip8 chip8;
    chip8_init(&chip8);

    create_test_rom(ROM_CAPACITY);

    assert(chip8_load_rom(&chip8, TEST_ROM_PATH) == 0);

    for (int i = 0; i < ROM_CAPACITY; i++) {
        assert(chip8.memory[PC_INIT + i] == (uint8_t)(i % 256));
    }

    assert(chip8.memory[CHIP8_MEMORY_SIZE - 1] ==
           (uint8_t)((ROM_CAPACITY - 1) % 256));

    puts("[PASS] Maximum ROM size");
}

/* Verify that oversized ROMs are rejected without modifying memory. */
static void test_oversized_rom(void)
{
    Chip8 chip8;
    chip8_init(&chip8);

    create_test_rom(ROM_CAPACITY + 1);

    assert(chip8_load_rom(&chip8, TEST_ROM_PATH) != 0);

    for (int i = PC_INIT; i < CHIP8_MEMORY_SIZE; i++) {
        assert(chip8.memory[i] == 0);
    }

    assert(chip8.pc == PC_INIT);

    puts("[PASS] Oversized ROM rejection");
}

/* Verify that an empty ROM is rejected. */
static void test_empty_rom(void)
{
    Chip8 chip8;
    chip8_init(&chip8);

    create_test_rom(0);

    assert(chip8_load_rom(&chip8, TEST_ROM_PATH) != 0);

    puts("[PASS] Empty ROM rejection");
}

int main(void)
{
    puts("Running CHIP-8 ROM tests...\n");

    test_valid_rom();
    test_missing_rom();
    test_maximum_rom();
    test_oversized_rom();
    test_empty_rom();

    remove(TEST_ROM_PATH);

    puts("\nAll ROM tests passed!");

    return 0;
}