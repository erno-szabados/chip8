#ifndef __CHIP8_H
#define __CHIP8_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>

// Instruction time
#define CHIP8E_CMD_TS_DELAY_MS 100

// bytes
#define CHIP8E_MEM_SIZE 4096

// Masks for valid memory and register arguments
#define CHIP8E_MEM_MASK(n) ((n) & 0xFFF)
#define CHIP8E_REG_MASK(n) ((n) & 0xF)

// TODO Review endian operations on different (L/B) endian archs.
#define CHIP8_ENDIAN_MASK_LSB 0x01
#define CHIP8_ENDIAN_MASK_MSB 0x80

// must be a multiple of 8
#define CHIP8E_MEMDUMP_BLOCK_SIZE 32

// words, stores pc
#define CHIP8E_STACK_SIZE 16

// Screen resolution, some models has more
#define CHIP8E_XRES 64
#define CHIP8E_YRES 32

// Used to clear memory
#define CHIP8_EMPTY_WORD 0xFFFF
#define CHIP8_EMPTY_BYTE 0xFF

// Memory Map
#define CHIP8E_MEM_OFFSET_PROGRAM_END       0xFFF
#define CHIP8E_MEM_OFFSET_PROGRAM_START     0x200
#define CHIP8E_MEM_OFFSET_INTERPRETER_END   0x1FF
#define CHIP8E_MEM_OFFSET_SPRITE_START      0x050
#define CHIP8E_MEM_OFFSET_SPRITE_END        0x0A0
#define CHIP8E_MEM_OFFSET_INTERPRETER_START 0x000

// Register names
#define V0 0x0
#define V1 0x1
#define V2 0x2
#define V3 0x3
#define V4 0x4
#define V5 0x5
#define V6 0x6
#define V7 0x7
#define V8 0x8
#define V9 0x9
#define VA 0xA
#define VB 0xB
#define VC 0xC
#define VD 0xD
#define VE 0xE
// Flag register - programs should not use this
#define VF 0xF

// used for traps
typedef enum {CHIP_STATE_NORMAL, CHIP_STATE_EXCEPTION, CHIP_STATE_EXIT} chip8_state_t;

// Processor, Memory and Video Status
typedef struct {
    uint16_t opcode;
    uint8_t memory[CHIP8E_MEM_SIZE];
    // It would be sufficient to store 1 bit per pixel, but
    // then showing it would require expensive bit->byte extensions
    // Modern GUI libraries usually do not deal well with 1-bpp depth.
    uint8_t video_buffer[CHIP8E_XRES * CHIP8E_YRES];
    bool video_dirty;
    uint16_t stack[CHIP8E_STACK_SIZE];
    // v0..15 general purpose register
    uint8_t V[16];
    // Index Register
    uint16_t I;
    // Program Counter
    uint16_t PC;
    // Stack Pointer
    uint8_t SP;
    // Delay Timer, Sound Timer
    uint8_t DT, ST;
    chip8_state_t state;
    // Instruction delay timespec
    struct timespec cmd_delay_ts;
} chip8_t, *chip8_p;

// Initialize the emulator
void chip8_init(chip8_p chip);
// Fetch, decode, execute...
void chip8_cycle(chip8_p chip);
// Load program from memory block to program area
void chip8_load_program_block(chip8_p chip, uint8_t *buf, uint16_t size);
// Display trapping info
void chip8_trap(chip8_p chip);
// Display memory dump
void chip8_memdump(chip8_p chip, uint16_t addr);
// Copy data block to emulator memory.
void chip8_block_to_mem(chip8_p chip, uint16_t offset, uint8_t *buf, uint16_t size);
// Copy emulator memory to data block.
void chip8_mem_to_block(chip8_p chip, uint16_t offset, uint8_t *buf, uint16_t size);
// Read file to passed block and set size to number of bytes read.
int chip8_file_to_block(chip8_p chip, char *filename, uint8_t *buf, uint16_t *size);

// A glorified switch case
void chip8_interpret_cmd(chip8_p chip, uint16_t cmd);

#endif //__CHIP8_H

