#include <stdio.h>
#include <string.h>
#include "chip8.h"

int chip8_fetch(Chip8 *chip8, uint16_t *opcode) {
    if (chip8->pc >= CHIP8_MEMORY_SIZE -1) {
        fprintf(stderr, "Error: PC out of bounds\n");
        return 1;
    }

    // Instructions are two bytes, stored most-significant byte first.
    *opcode = (chip8->memory[chip8->pc] << 8) | chip8->memory[chip8->pc + 1];
   
    chip8->pc += 2;

    return 0;
}

void chip8_execute(Chip8 *chip8, uint16_t opcode)
{
    // Decode register indices, the immediate byte, and the 12-bit address.
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t y = (opcode & 0x00F0) >> 4;
    uint8_t kk = opcode & 0x00FF;
    uint16_t nnn = opcode & 0x0FFF;

    uint8_t carry;

    switch (opcode >> 12) {
        case 0x0:
            switch (opcode) {
                case 0x00E0:
                    memset(chip8->gfx, 0, sizeof(chip8->gfx));
                    break;
                
                case 0x00EE:
                    if (chip8->sp > 0) {
                        chip8->pc = chip8->stack[--chip8->sp];
                    }
                    else{
                        fprintf(stderr, "Error: Stack underflow\n");
                    }
                    break;    
                
                default:
                    break;
            }
            
            break;

        case 0x1:
            chip8->pc = nnn;
            break;
        
        case 0x2:
            if (chip8->sp < 16) {
                chip8->stack[chip8->sp++] = chip8->pc;
                chip8->pc = nnn;
            }
            else{
                fprintf(stderr, "Error: Stack overflow\n");
            }
            
            break; 
        
        case 0x3:
            if (chip8->V[x] == kk) {
                chip8->pc += 2;
            }
            
            break;  
        
        case 0x4:
            if (chip8->V[x] != kk) {
                chip8->pc += 2;
            }
            
            break;     
        
        case 0x5:
            if (((opcode & 0xF) == 0) && (chip8->V[x] == chip8->V[y])) {
                chip8->pc += 2;
            }
            
            break;      

        case 0x6:
            chip8->V[x] = kk;
            break;

        case 0x7:
            chip8->V[x] += kk;
            break;

        case 0x8:
            switch (opcode & 0x000F) {
                case 0x0:
                    chip8->V[x] = chip8->V[y];
                    break;
                
                case 0x1:
                    chip8->V[x] |= chip8->V[y];
                    break;
                
                case 0x2:
                    chip8->V[x] &= chip8->V[y];
                    break;
                
                case 0x3:
                    chip8->V[x] ^= chip8->V[y];
                    break;
                
                case 0x4:
                    carry = chip8->V[x] > (UINT8_MAX - chip8->V[y]) ? 1 : 0;
                    
                    chip8->V[x] += chip8->V[y];
                    chip8->V[0xF] = carry;
                    
                    break;
                
                case 0x5:
                    carry = chip8->V[x] > chip8->V[y] ? 1 : 0;

                    chip8->V[x] -= chip8->V[y];
                    chip8->V[0xF] = carry;
                    break;
                
                case 0x6:
                    carry = chip8->V[x] & 0x1 ? 1 : 0;
                    
                    chip8->V[x] >>= 1;
                    chip8->V[0xF] = carry;
                    break;
                
                case 0x7:
                    carry = chip8->V[x] < chip8->V[y] ? 1 : 0;

                    chip8->V[x] = chip8->V[y] - chip8->V[x];
                    chip8->V[0xF] = carry;
                    break;
                
                case 0xE:
                    carry = (chip8->V[x] >> 7) & 0x1 ? 1 : 0;

                    chip8->V[x] <<= 1;
                    chip8->V[0xF] = carry;
                    break;
                   
                
                default:
                    break;
                }
            
            break;
        
        case 0x9:
            if (((opcode & 0xF) == 0) && (chip8->V[x] != chip8->V[y])) {
                chip8->pc += 2;
            }
            
            break;
        
        case 0xA:
            chip8->I = nnn;
            break;
        
        case 0xB:
            chip8->pc = (nnn) + chip8->V[0];
            break;
            
        case 0xC:
            chip8->V[x] = (rand() % 256) & (kk);
            break;
            
        case 0xD:
            if (chip8->I + (opcode & 0xF) <= CHIP8_MEMORY_SIZE) {
                // Sprite bits are XORed onto the display; VF reports erased pixels.
                int x_pos, y_pos, idx, collision_flag = 0;
                uint8_t sprite_byte, sprite_bit;
          
                for (int i = 0; i < (opcode & 0xF); i++)
                {
                    y_pos = (chip8->V[y] + i) % 32;
                    
                    sprite_byte = chip8->memory[chip8->I + i];
                    
                    for (int j = 0; j < 8; j++) 
                    {
                        x_pos = (chip8->V[x] + j) % 64;

                        sprite_bit = (sprite_byte >> (7 - j)) & 1;

                        idx = y_pos * 64 + x_pos;

                        if (sprite_bit == 1 && chip8->gfx[idx] == 1) {
                            collision_flag = 1;
                        }

                        chip8->gfx[idx] ^= sprite_bit;
                    }   
                }

                chip8->V[0xF] = collision_flag;
            }
            else {
                fprintf(stderr, "Error: Memory out of bounds for instruction Dxyn\n");
            }
            
                
            break;
            
        case 0xE:
            if (chip8->V[x] < 16) {
                switch (opcode & 0xFF) {
                    case 0x9E:
                        if (chip8->key[chip8->V[x]] == 1) {
                            chip8->pc += 2;
                        }
                        break;
                    
                    case 0xA1:
                        if (chip8->key[chip8->V[x]] == 0) {
                            chip8->pc += 2;
                        }
                        break;    
                    
                    default:
                        break;
                }
            }
            else {
                fprintf(stderr, "Error: Value in Vx doesn't match existing keys in instruction Ex9E/ExA1\n");
            }
            
            break;
            
        case 0xF:
            switch (opcode & 0xFF) {
                case 0x07:
                    chip8->V[x] = chip8->dt;
                    break;
                
                case 0x0A:
                    // Wait for a pressed key to be released before advancing.
                    if (chip8->held_key < 16) {
                        if (!chip8->key[chip8->held_key]) {
                            chip8->V[x] = chip8->held_key;
                            chip8->held_key = CHIP8_NO_KEY;
                            return;
                        }
                    }
                    else {
                        for (int i = 0; i < 16; i++) {
                            if (chip8->key[i]) {
                                chip8->held_key = i;
                                break;
                            }
                        }
                    }

                    // Fetch already advanced PC; repeat this instruction while waiting.
                    chip8->pc -= 2;
                    
                    break;
                    
                case 0x15:
                    chip8->dt = chip8->V[x];
                    break;
                    
                case 0x18:
                    chip8->st = chip8->V[x];
                    break;
                
                case 0x1E:
                    chip8->I += chip8->V[x];
                    break;
                    
                case 0x29:
                    chip8->I = 0x050 + (chip8->V[x] * 5);
                    break;
                    
                case 0x33:
                    // Store the decimal digits of Vx as three separate bytes.
                    if ((chip8->I + 2) < CHIP8_MEMORY_SIZE) {
                        chip8->memory[chip8->I] = chip8->V[x] / 100;
                        chip8->memory[chip8->I + 1] = (chip8->V[x] / 10) % 10;
                        chip8->memory[chip8->I + 2] = chip8->V[x] % 10;
                    }
                    else {
                        fprintf(stderr, "Error: Memory out of bounds for instruction Fx33\n");
                    }
                    
                    break;
                    
                case 0x55:
                    if (chip8->I + x < CHIP8_MEMORY_SIZE) { 
                        for (int i = 0; i <= x; i++)
                        {
                            chip8->memory[chip8->I + i] = chip8->V[i];
                        }
                    }   
                    else {
                        fprintf(stderr, "Error: Memory out of bounds for instruction Fx55\n");
                    }
                    
                    break;
                    
                case 0x65:
                    if (chip8->I + x < CHIP8_MEMORY_SIZE) { 
                        for (int i = 0; i <= x; i++)
                        {
                            chip8->V[i] = chip8->memory[chip8->I + i];
                        }
                    }   
                    else {
                        fprintf(stderr, "Error: Memory out of bounds for instruction Fx65\n");
                    }

                    break;    
                
                default:
                    break;
            }
            break;    

        default:
            fprintf(stderr, "Error: Unsupport opcode %x\n", opcode);
            break;
    }
}