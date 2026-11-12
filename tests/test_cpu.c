#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "chip8.h"

/*
 * CPU test suite.
 *
 * Assumes the following functions are declared in chip8.h:
 *
 * int chip8_fetch(Chip8 *chip8, uint16_t *opcode);
 * void chip8_execute(Chip8 *chip8, uint16_t opcode);
 *
 * Each test initializes a fresh CHIP-8 machine.
 */


/* ---------------------------------------------------------
 * Instruction fetching
 * --------------------------------------------------------- */

static void test_fetch(void)
{
    Chip8 chip8;
    uint16_t opcode = 0;

    chip8_init(&chip8);

    chip8.memory[PC_INIT]     = 0x63;
    chip8.memory[PC_INIT + 1] = 0xAF;

    assert(chip8_fetch(&chip8, &opcode) == 0);

    assert(opcode == 0x63AF);
    assert(chip8.pc == PC_INIT + 2);

    puts("[PASS] Instruction fetching");
}


/* ---------------------------------------------------------
 * 00E0 - Clear display
 * --------------------------------------------------------- */

static void test_clear_display(void)
{
    Chip8 chip8;
    chip8_init(&chip8);

    memset(chip8.gfx, 1, sizeof(chip8.gfx));

    chip8_execute(&chip8, 0x00E0);

    for (size_t i = 0; i < sizeof(chip8.gfx); i++) {
        assert(chip8.gfx[i] == 0);
    }

    puts("[PASS] Clear display");
}


/* ---------------------------------------------------------
 * 1nnn - Jump
 * --------------------------------------------------------- */

static void test_jump(void)
{
    Chip8 chip8;
    chip8_init(&chip8);

    chip8_execute(&chip8, 0x1345);

    assert(chip8.pc == 0x345);

    puts("[PASS] Jump");
}


/* ---------------------------------------------------------
 * 2nnn / 00EE - Call and return
 * --------------------------------------------------------- */

static void test_call_return(void)
{
    Chip8 chip8;
    chip8_init(&chip8);

    /*
     * Simulate the state after fetching a CALL instruction.
     * Fetching has already advanced PC by 2.
     */
    chip8.pc = 0x202;

    chip8_execute(&chip8, 0x2300);

    assert(chip8.pc == 0x300);
    assert(chip8.sp == 1);
    assert(chip8.stack[0] == 0x202);

    chip8_execute(&chip8, 0x00EE);

    assert(chip8.pc == 0x202);
    assert(chip8.sp == 0);

    puts("[PASS] Call and return");
}


/* ---------------------------------------------------------
 * 3xkk / 4xkk - Conditional immediate skips
 * --------------------------------------------------------- */

static void test_immediate_skips(void)
{
    Chip8 chip8;
    chip8_init(&chip8);

    chip8.V[3] = 0x42;

    chip8.pc = 0x202;
    chip8_execute(&chip8, 0x3342);
    assert(chip8.pc == 0x204);

    chip8.pc = 0x202;
    chip8_execute(&chip8, 0x3343);
    assert(chip8.pc == 0x202);

    chip8.pc = 0x202;
    chip8_execute(&chip8, 0x4343);
    assert(chip8.pc == 0x204);

    chip8.pc = 0x202;
    chip8_execute(&chip8, 0x4342);
    assert(chip8.pc == 0x202);

    puts("[PASS] Immediate conditional skips");
}


/* ---------------------------------------------------------
 * 5xy0 / 9xy0 - Register conditional skips
 * --------------------------------------------------------- */

static void test_register_skips(void)
{
    Chip8 chip8;
    chip8_init(&chip8);

    chip8.V[1] = 10;
    chip8.V[2] = 10;
    chip8.V[3] = 20;

    chip8.pc = 0x202;
    chip8_execute(&chip8, 0x5120);
    assert(chip8.pc == 0x204);

    chip8.pc = 0x202;
    chip8_execute(&chip8, 0x5130);
    assert(chip8.pc == 0x202);

    chip8.pc = 0x202;
    chip8_execute(&chip8, 0x9130);
    assert(chip8.pc == 0x204);

    chip8.pc = 0x202;
    chip8_execute(&chip8, 0x9120);
    assert(chip8.pc == 0x202);

    /* Invalid final nibbles must not trigger skips. */
    chip8.pc = 0x202;
    chip8_execute(&chip8, 0x5121);
    assert(chip8.pc == 0x202);

    chip8.pc = 0x202;
    chip8_execute(&chip8, 0x9131);
    assert(chip8.pc == 0x202);

    puts("[PASS] Register conditional skips");
}


/* ---------------------------------------------------------
 * 6xkk / 7xkk - Load and add immediate
 * --------------------------------------------------------- */

static void test_immediate_arithmetic(void)
{
    Chip8 chip8;
    chip8_init(&chip8);

    chip8_execute(&chip8, 0x63AF);
    assert(chip8.V[3] == 0xAF);

    chip8_execute(&chip8, 0x7301);
    assert(chip8.V[3] == 0xB0);

    /* 8-bit arithmetic wraps around. */
    chip8_execute(&chip8, 0x63FF);
    chip8_execute(&chip8, 0x7302);

    assert(chip8.V[3] == 0x01);

    puts("[PASS] Immediate arithmetic");
}


/* ---------------------------------------------------------
 * 8xy0 / 8xy1 / 8xy2 / 8xy3 - Register operations
 * --------------------------------------------------------- */

static void test_register_operations(void)
{
    Chip8 chip8;
    chip8_init(&chip8);

    chip8.V[1] = 0xAA;
    chip8.V[2] = 0x0F;

    chip8_execute(&chip8, 0x8120);
    assert(chip8.V[1] == 0x0F);

    chip8.V[1] = 0xA0;
    chip8_execute(&chip8, 0x8121);
    assert(chip8.V[1] == 0xAF);

    chip8.V[1] = 0xAA;
    chip8_execute(&chip8, 0x8122);
    assert(chip8.V[1] == 0x0A);

    chip8.V[1] = 0xAA;
    chip8_execute(&chip8, 0x8123);
    assert(chip8.V[1] == 0xA5);

    puts("[PASS] Register operations");
}


/* ---------------------------------------------------------
 * 8xy4 - Addition with carry
 * --------------------------------------------------------- */

static void test_add_with_carry(void)
{
    Chip8 chip8;
    chip8_init(&chip8);

    chip8.V[1] = 250;
    chip8.V[2] = 10;

    chip8_execute(&chip8, 0x8124);

    assert(chip8.V[1] == 4);
    assert(chip8.V[0xF] == 1);

    chip8.V[1] = 10;
    chip8.V[2] = 20;

    chip8_execute(&chip8, 0x8124);

    assert(chip8.V[1] == 30);
    assert(chip8.V[0xF] == 0);

    puts("[PASS] Addition with carry");
}


/* ---------------------------------------------------------
 * 8xy5 / 8xy7 - Subtraction
 * --------------------------------------------------------- */

static void test_subtraction(void)
{
    Chip8 chip8;
    chip8_init(&chip8);

    chip8.V[1] = 20;
    chip8.V[2] = 10;

    chip8_execute(&chip8, 0x8125);

    assert(chip8.V[1] == 10);
    assert(chip8.V[0xF] == 1);

    chip8.V[1] = 20;
    chip8.V[2] = 10;

    chip8_execute(&chip8, 0x8127);

    assert(chip8.V[1] == 246);
    assert(chip8.V[0xF] == 0);

    chip8.V[1] = 10;
    chip8.V[2] = 20;

    chip8_execute(&chip8, 0x8127);

    assert(chip8.V[1] == 10);
    assert(chip8.V[0xF] == 1);

    puts("[PASS] Subtraction");
}


/* ---------------------------------------------------------
 * 8xy6 / 8xyE - Bit shifts
 * --------------------------------------------------------- */

static void test_shifts(void)
{
    Chip8 chip8;
    chip8_init(&chip8);

    chip8.V[1] = 0x05;

    chip8_execute(&chip8, 0x8106);

    assert(chip8.V[1] == 0x02);
    assert(chip8.V[0xF] == 1);

    chip8.V[1] = 0x81;

    chip8_execute(&chip8, 0x810E);

    assert(chip8.V[1] == 0x02);
    assert(chip8.V[0xF] == 1);

    puts("[PASS] Bit shifts");
}


/* ---------------------------------------------------------
 * Annn / Bnnn - Index register and offset jump
 * --------------------------------------------------------- */

static void test_index_and_offset_jump(void)
{
    Chip8 chip8;
    chip8_init(&chip8);

    chip8_execute(&chip8, 0xA345);
    assert(chip8.I == 0x345);

    chip8.V[0] = 5;

    chip8_execute(&chip8, 0xB300);
    assert(chip8.pc == 0x305);

    puts("[PASS] Index register and offset jump");
}


/* ---------------------------------------------------------
 * Cxkk - Random number with mask
 * --------------------------------------------------------- */

static void test_random_mask(void)
{
    Chip8 chip8;
    chip8_init(&chip8);

    /*
     * A random result cannot be predicted, but its mask
     * can be verified.
     */
    for (int i = 0; i < 100; i++) {
        chip8_execute(&chip8, 0xC3F0);

        assert((chip8.V[3] & 0x0F) == 0);
    }

    chip8_execute(&chip8, 0xC300);
    assert(chip8.V[3] == 0);

    puts("[PASS] Random masking");
}


/* ---------------------------------------------------------
 * Ex9E / ExA1 - Keypad skips
 * --------------------------------------------------------- */

static void test_keypad_skips(void)
{
    Chip8 chip8;
    chip8_init(&chip8);

    chip8.V[3] = 0xA;
    chip8.key[0xA] = 1;

    chip8.pc = 0x202;
    chip8_execute(&chip8, 0xE39E);
    assert(chip8.pc == 0x204);

    chip8.pc = 0x202;
    chip8_execute(&chip8, 0xE3A1);
    assert(chip8.pc == 0x202);

    chip8.key[0xA] = 0;

    chip8.pc = 0x202;
    chip8_execute(&chip8, 0xE3A1);
    assert(chip8.pc == 0x204);

    puts("[PASS] Keypad skips");
}


/* ---------------------------------------------------------
 * Fx07 / Fx15 / Fx18 - Timers
 * --------------------------------------------------------- */

static void test_timers(void)
{
    Chip8 chip8;
    chip8_init(&chip8);

    chip8.V[3] = 42;

    chip8_execute(&chip8, 0xF315);
    assert(chip8.dt == 42);

    chip8_execute(&chip8, 0xF318);
    assert(chip8.st == 42);

    chip8.V[3] = 0;

    chip8_execute(&chip8, 0xF307);
    assert(chip8.V[3] == 42);

    puts("[PASS] Timer register operations");
}


/* ---------------------------------------------------------
 * Fx1E - Add to index register
 * --------------------------------------------------------- */

static void test_index_add(void)
{
    Chip8 chip8;
    chip8_init(&chip8);

    chip8.I = 0x300;
    chip8.V[2] = 5;

    chip8_execute(&chip8, 0xF21E);

    assert(chip8.I == 0x305);

    puts("[PASS] Index addition");
}


/* ---------------------------------------------------------
 * Fx33 - Binary-coded decimal
 * --------------------------------------------------------- */

static void test_bcd(void)
{
    Chip8 chip8;
    chip8_init(&chip8);

    chip8.I = 0x300;
    chip8.V[2] = 237;

    chip8_execute(&chip8, 0xF233);

    assert(chip8.memory[0x300] == 2);
    assert(chip8.memory[0x301] == 3);
    assert(chip8.memory[0x302] == 7);

    puts("[PASS] BCD conversion");
}


/* ---------------------------------------------------------
 * Fx55 / Fx65 - Register and memory transfers
 * --------------------------------------------------------- */

static void test_memory_transfers(void)
{
    Chip8 chip8;
    chip8_init(&chip8);

    chip8.I = 0x300;

    chip8.V[0] = 10;
    chip8.V[1] = 20;
    chip8.V[2] = 30;

    chip8_execute(&chip8, 0xF255);

    assert(chip8.memory[0x300] == 10);
    assert(chip8.memory[0x301] == 20);
    assert(chip8.memory[0x302] == 30);

    /* Clear registers before restoring them. */
    chip8.V[0] = 0;
    chip8.V[1] = 0;
    chip8.V[2] = 0;

    chip8_execute(&chip8, 0xF265);

    assert(chip8.V[0] == 10);
    assert(chip8.V[1] == 20);
    assert(chip8.V[2] == 30);

    puts("[PASS] Register/memory transfers");
}

static void test_draw_sprite(void)
{
    Chip8 chip8;
    chip8_init(&chip8);

    chip8.I = 0x300;

    /* Three-row sprite. */
    chip8.memory[0x300] = 0xF0;  /* 11110000 */
    chip8.memory[0x301] = 0x90;  /* 10010000 */
    chip8.memory[0x302] = 0xF0;  /* 11110000 */

    chip8.V[1] = 5;
    chip8.V[2] = 3;

    chip8_execute(&chip8, 0xD123);

    /* First row: four illuminated pixels. */
    for (int x = 5; x < 9; x++) {
        assert(chip8.gfx[3 * 64 + x] == 1);
    }

    /* Second row: only the first and fourth pixels. */
    assert(chip8.gfx[4 * 64 + 5] == 1);
    assert(chip8.gfx[4 * 64 + 6] == 0);
    assert(chip8.gfx[4 * 64 + 7] == 0);
    assert(chip8.gfx[4 * 64 + 8] == 1);

    assert(chip8.V[0xF] == 0);

    /* Drawing the same sprite again should erase it. */
    chip8_execute(&chip8, 0xD123);

    assert(chip8.V[0xF] == 1);

    for (int i = 0; i < 64 * 32; i++) {
        assert(chip8.gfx[i] == 0);
    }

    puts("[PASS] Sprite drawing and XOR collision");
}

static void test_sprite_wrapping(void)
{
    Chip8 chip8;
    chip8_init(&chip8);

    chip8.I = 0x300;

    chip8.memory[0x300] = 0xFF;
    chip8.memory[0x301] = 0xFF;

    chip8.V[1] = 62;
    chip8.V[2] = 31;

    chip8_execute(&chip8, 0xD122);

    /* Horizontal wrapping. */
    assert(chip8.gfx[31 * 64 + 62] == 1);
    assert(chip8.gfx[31 * 64 + 63] == 1);
    assert(chip8.gfx[31 * 64 + 0] == 1);
    assert(chip8.gfx[31 * 64 + 5] == 1);

    /* Vertical wrapping. */
    assert(chip8.gfx[0 * 64 + 62] == 1);
    assert(chip8.gfx[0 * 64 + 63] == 1);
    assert(chip8.gfx[0 * 64 + 0] == 1);
    assert(chip8.gfx[0 * 64 + 5] == 1);

    assert(chip8.V[0xF] == 0);

    puts("[PASS] Sprite wrapping");
}

static void test_wait_for_key_no_input(void)
{
    Chip8 chip8;
    chip8_init(&chip8);

    /*
     * Simulate the PC after fetching Fx0A.
     * Fetching advances the PC by two bytes.
     */
    chip8.pc = 0x202;
    chip8.V[1] = 0xFF;

    chip8_execute(&chip8, 0xF10A);

    assert(chip8.pc == 0x200);
    assert(chip8.V[1] == 0xFF);

    puts("[PASS] Fx0A - No key pressed");
}

static void test_wait_for_key_pressed(void)
{
    Chip8 chip8;
    chip8_init(&chip8);

    chip8.pc = 0x202;
    chip8.key[0xA] = 1;

    chip8_execute(&chip8, 0xF10A);

    assert(chip8.V[1] == 0xA);
    assert(chip8.pc == 0x202);

    puts("[PASS] Fx0A - Key pressed");
}

static void test_wait_for_key_multiple(void)
{
    Chip8 chip8;
    chip8_init(&chip8);

    chip8.pc = 0x202;

    chip8.key[0xC] = 1;
    chip8.key[0x7] = 1;
    chip8.key[0x2] = 1;

    chip8_execute(&chip8, 0xF10A);

    assert(chip8.V[1] == 0x2);
    assert(chip8.pc == 0x202);

    puts("[PASS] Fx0A - Multiple keys pressed");
}

/* ---------------------------------------------------------
 * Test runner
 * --------------------------------------------------------- */

int main(void)
{
    puts("Running CHIP-8 CPU tests...\n");

    test_fetch();
    test_clear_display();
    test_jump();
    test_call_return();
    test_immediate_skips();
    test_register_skips();
    test_immediate_arithmetic();
    test_register_operations();
    test_add_with_carry();
    test_subtraction();
    test_shifts();
    test_index_and_offset_jump();
    test_random_mask();
    test_keypad_skips();
    test_timers();
    test_index_add();
    test_bcd();
    test_memory_transfers();
    test_sprite_wrapping();
    test_draw_sprite();
    test_wait_for_key_no_input();
    test_wait_for_key_pressed();
    test_wait_for_key_multiple();

    puts("\nAll CPU tests passed!");

    return 0;
}