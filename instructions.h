#ifndef __INSTRUCTIONS_H
#define __INSTRUCTIONS_H

#include "chip8.h"
#include <stdlib.h>

/**
 * Instruction set for CHIP8 Virtual Machine.
 *
 * All instructions are 2-byte, MSB first. In memory, the first byte
 * of each instruction should be on even addresses. E.g. sprite data
 * should be padded accordingly.
 *
 * n - 4-bit value
 * nn - 8-bit value
 * nnn - 12-bit value
 **/

// Jump to a machine code routine at nnn.
 void i_sys(chip8_p chip, uint16_t addr)
{
    printf("System call requested to %04X.\n", CHIP8E_MEM_MASK(addr));
    chip->PC += 2;
    // unimplemented
}

// Clear the display.
 void i_cls(chip8_p chip)
{
    printf("%04X: CLS\n", chip->PC);
    for (int i = 0; i < (CHIP8E_XRES * CHIP8E_YRES); i++) {
        chip->video_buffer[i] = 0x0;
    }
    chip->PC += 2;
    chip->video_dirty = true;
    // TODO Display API dependent
}

// Return from a subroutine.
 void i_ret(chip8_p chip)
{
    printf("%04X: RET \n", chip->PC);
    chip8_stack_pop(chip, &(chip->PC));
    chip->PC += 2;
}

// Sets the program counter to nnn.
 void i_jp(chip8_p chip, uint16_t addr)
{
    printf("%04X: JP   %04x\n", chip->PC, addr);
    chip->PC = CHIP8E_MEM_MASK(addr);
}

// Call subroutine at nnn.
 void i_call(chip8_p chip, uint16_t addr)
{
    printf("%04X: CALL %04x\n", chip->PC, addr);
    chip8_stack_push(chip, chip->PC);
    chip->PC = CHIP8E_MEM_MASK(addr);
}

// Skip next instruction if Vx = nn.
 void i_sevxb(chip8_p chip, uint8_t reg, uint8_t b)
{
    printf("%04X: SE   V%02X %02x\n", chip->PC, reg, b);

    if (chip->V[CHIP8E_REG_MASK(reg)] == b)
        chip->PC += 4;
    else
        chip->PC += 2;
}

// Skip next instruction if Vx != nn.
 void i_snevxb(chip8_p chip, uint8_t reg, uint8_t b)
{
    printf("%04X: SNE  V%02X %02x\n", chip->PC, reg, b);
    if (chip->V[CHIP8E_REG_MASK(reg)] != b)
        chip->PC += 4;
    else
        chip->PC += 2;
}

// Skip next instruction if Vx = Vy.
 void i_sevxvy(chip8_p chip, uint8_t regx, uint8_t regy)
{
    printf("%04X: SE   V%02X V%02X\n", chip->PC, regx, regy);

    if (chip->V[CHIP8E_REG_MASK(regx)] == chip->V[CHIP8E_REG_MASK(regy)])
        chip->PC += 4;
    else
        chip->PC += 2;
}

// puts the value kk into register Vx
 void i_ldvxb(chip8_p chip, uint8_t reg, uint8_t b)
{
    printf("%04X: LD   V%02X %02x\n", chip->PC, reg, b);
    chip->V[CHIP8E_REG_MASK(reg)] = b;
    chip->PC += 2;
}

// adds the value kk to register Vx
 void i_addvxb(chip8_p chip, uint8_t reg, uint8_t b)
{
    printf("%04X: ADD  V%02X %02x\n", chip->PC, reg, b);
    chip->V[CHIP8E_REG_MASK(reg)] += b;
    chip->PC += 2;
}

// load register Vy to register Vx
 void i_ldvxvy(chip8_p chip, uint8_t regx, uint8_t regy)
{
    printf("%04X: LD   V%02X V%02X\n", chip->PC, regx, regy);
    chip->V[CHIP8E_REG_MASK(regx)] = chip->V[CHIP8E_REG_MASK(regy)];
    chip->PC += 2;
}

// bitwise or register Vy and register Vx
 void i_orvxvy(chip8_p chip, uint8_t regx, uint8_t regy)
{
    printf("%04X: OR   V%02X V%02X\n", chip->PC, regx, regy);
    chip->V[CHIP8E_REG_MASK(regx)] |= chip->V[CHIP8E_REG_MASK(regy)];
    chip->PC += 2;
}

// bitwise and register Vy and register Vx
 void i_andvxvy(chip8_p chip, uint8_t regx, uint8_t regy)
{
    printf("%04X: AND  V%02X V%02X\n", chip->PC, regx, regy);
    chip->V[CHIP8E_REG_MASK(regx)] &= chip->V[CHIP8E_REG_MASK(regy)];
    chip->PC += 2;
}

// bitwise xor register Vy and register Vx
 void i_xorvxvy(chip8_p chip, uint8_t regx, uint8_t regy)
{
    printf("%04X: XOR  V%02X V%02X\n", chip->PC, regx, regy);
    chip->V[CHIP8E_REG_MASK(regx)] ^= chip->V[CHIP8E_REG_MASK(regy)];
    chip->PC += 2;
}

// add register Vy and register Vx and store result in Vx, set VF = carry
 void i_addvxvy(chip8_p chip, uint8_t regx, uint8_t regy)
{
    printf("%04X: ADD  V%02X V%02X\n", chip->PC, regx, regy);
    chip->V[VF] = (chip->V[CHIP8E_REG_MASK(regx)] + chip->V[CHIP8E_REG_MASK(regy)] > 0xFF) ? 1 : 0;
    chip->V[CHIP8E_REG_MASK(regx)] += chip->V[CHIP8E_REG_MASK(regy)];
    chip->PC += 2;
}

// substract register Vy from register Vx and store result in Vx, set VF = borrow
 void i_subvxvy(chip8_p chip, uint8_t regx, uint8_t regy)
{
    printf("%04X: SUB  V%02X V%02X\n", chip->PC, regx, regy);
    chip->V[VF] = (chip->V[CHIP8E_REG_MASK(regx)] > chip->V[CHIP8E_REG_MASK(regy)]) ? 1 : 0;
    chip->V[CHIP8E_REG_MASK(regx)] -= chip->V[CHIP8E_REG_MASK(regy)];
    chip->PC += 2;
}

// If the least-significant bit of Vx is 1, then VF is set to 1, otherwise 0. Then Vx is divided by 2.
 void i_shrvx(chip8_p chip, uint8_t regx)
{
    printf("%04X: SHR  V%02X\n", chip->PC, regx);
    // LSB -> endianness!
    chip->V[VF] = (chip->V[CHIP8E_REG_MASK(regx)] & CHIP8_ENDIAN_MASK_LSB);
    chip->V[CHIP8E_REG_MASK(regx)] >>= 1;
    chip->PC += 2;
}

// substract register Vy from register Vx and store result in Vx, set VF = borrow
 void i_subnvxvy(chip8_p chip, uint8_t regx, uint8_t regy)
{
    printf("%04X: SUBN V%02X V%02X\n", chip->PC, regx, regy);
    chip->V[VF] = (chip->V[CHIP8E_REG_MASK(regy)] > chip->V[CHIP8E_REG_MASK(regx)]) ? 1 : 0;
    chip->V[CHIP8E_REG_MASK(regx)] = chip->V[CHIP8E_REG_MASK(regy)] - chip->V[CHIP8E_REG_MASK(regx)];
    chip->PC += 2;
}

// If the least-significant bit of Vx is 1, then VF is set to 1, otherwise 0. Then Vx is divided by 2.
 void i_shlvx(chip8_p chip, uint8_t regx)
{
    printf("%04X: SHL  V%02X\n", chip->PC, regx);
    // LSB -> endianness!
    chip->V[VF] = (chip->V[CHIP8E_REG_MASK(regx)] & CHIP8_ENDIAN_MASK_MSB);
    chip->V[CHIP8E_REG_MASK(regx)] <<= 1;
    chip->PC += 2;
}

// Skip next instruction if Vx != Vy.
 void i_snevxvy(chip8_p chip, uint8_t regx, uint8_t regy)
{
    printf("%04X: SNE  V%02X V%02X\n", chip->PC, regx, regy);
    if (chip->V[CHIP8E_REG_MASK(regx)] != chip->V[CHIP8E_REG_MASK(regy)])
        chip->PC += 4;
        else
        chip->PC += 2;
}

// load w to register I.
 void i_ldiw(chip8_p chip, uint16_t w)
{
    printf("%04X: LDI  %04x\n", chip->PC, w);
    chip->I = CHIP8E_MEM_MASK(w);
    chip->PC += 2;
}

//  Jump to location nnn + V0 (base relative)
 void i_jpv0w(chip8_p chip, uint16_t addr)
{
    printf("%04X: JP   V0 %04x\n", chip->PC, addr);
    chip->PC = chip->V[V0] + CHIP8E_MEM_MASK(addr);
}

//  Set Vx = random byte AND kk.
 void i_rndvxb(chip8_p chip, uint8_t regx, uint8_t b)
{
    printf("%04X: RND  V%02X %02x\n", chip->PC, regx, b);
    chip->V[CHIP8E_REG_MASK(regx)] = (rand() % 0xFF) & b;
    chip->PC += 2;
}

//  Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision.
 void i_drwvxvyn(chip8_p chip, uint8_t regx, uint8_t regy, uint8_t b)
{
    // Display API dependent
    // I points to memory location
    printf("%04X: DRW  V%02X V%02X %02x\n", chip->PC, regx, regy, b);
    //printf("Draw Sprite requested: I:%04X X:%02X Y:%02X bytes:%0X.\n",
      //  chip->I, chip->V[CHIP8E_REG_MASK(regx)], chip->V[CHIP8E_REG_MASK(regy)], b);
    // TODO
    chip->video_dirty = true;
    chip->PC += 2;
}

//  Skip next instruction if key with the value of Vx is pressed.
 void i_skpvx(chip8_p chip, uint8_t regx)
{
    printf("%04X: SKP  V%02X\n", chip->PC, regx);
    // Input API dependent
    //printf("Skip on keydown requested: VX:%02X.\n", chip->V[CHIP8E_REG_MASK(regx)]);
    // TODO
    chip->PC += 2;
}

//  Skip next instruction if key with the value of Vx is not pressed.
 void i_sknpvx(chip8_p chip, uint8_t regx)
{
    printf("%04X: SKNP V%02X\n", chip->PC, regx);
    // Input API dependent
    //printf("Skip on keyup requested: VX:%02X.\n", chip->V[CHIP8E_REG_MASK(regx)]);
    // TODO
    chip->PC += 2;
}

//  Set Vx = delay timer value.
 void i_ldvxdt(chip8_p chip, uint8_t regx)
{
    printf("%04X: LD   V%02X DT\n", chip->PC, regx);
    chip->V[CHIP8E_REG_MASK(regx)] = chip->DT;
    chip->PC += 2;
}

//  Set DT = Vx.
 void i_lddtvx(chip8_p chip, uint8_t regx)
{
    printf("%04X: LD   DT V%02X\n", chip->PC, regx);
    chip->DT = chip->V[CHIP8E_REG_MASK(regx)];
    chip->PC += 2;
}

//  Set ST = Vx.
 void i_ldstvx(chip8_p chip, uint8_t regx)
{
    printf("%04X: LD   ST V%02X\n", chip->PC, regx);
    // Sound API dependent
    chip->ST = chip->V[CHIP8E_REG_MASK(regx)];
    chip->PC += 2;
}

//  Wait for a key press, store the value of the key in Vx.
 void i_ldvxk(chip8_p chip, uint8_t regx)
{
    printf("%04X: LD   V%02X K\n", chip->PC, regx);
    // Input API dependent
    // TODO
    chip->V[CHIP8E_REG_MASK(regx)] = 0xFF;
    chip->PC += 2;
    //printf("Readkey requested: VX:%02X.\n", chip->V[CHIP8E_REG_MASK(regx)]);
}

// The values of I and Vx are added, and the results are stored in I.
 void i_addivx(chip8_p chip, uint8_t regx)
{
    printf("%04X: ADD  I V%02X\n", chip->PC, regx);
    chip->I += chip->V[CHIP8E_REG_MASK(regx)];
    chip->PC += 2;
}

// Set I = location of sprite for digit Vx.
// Register I points in memory to sprite representing value of VX as digit.
 void i_ldfvx(chip8_p chip, uint8_t regx)
{
    printf("%04X: LD   F V%02X\n", chip->PC, regx);
    // determine sprite address for sprite
    chip->I = CHIP8E_MEM_OFFSET_SPRITE_START + chip->V[CHIP8E_REG_MASK(regx)] * 5;
    chip->PC += 2;
}

// Store BCD representation of Vx in memory locations I, I+1, and I+2.
 void i_ldbvx(chip8_p chip, uint8_t regx) {
     printf("%04X: LD   B V%02X\n", chip->PC, regx);
    uint8_t n = chip->V[CHIP8E_REG_MASK(regx)];
    chip->memory[chip->I] = n / 100;
    chip->memory[chip->I + 1] = (n % 100) / 10;
    chip->memory[chip->I + 2] = (n % 100) % 10;
    chip->PC += 2;
}

// Store registers V0 through Vx in memory starting at location I.
void i_ldivx(chip8_p chip, uint8_t regx) {
    printf("%04X: LD   [I] V%02X\n", chip->PC, regx);
    for (int i = 0; i < CHIP8E_REG_MASK(regx); i++)
        chip->memory[chip->I + i] = chip->V[i];
    chip->PC += 2;
}

//  Read registers V0 through Vx from memory starting at location I.
void i_ldvxi(chip8_p chip, uint8_t regx) {
    printf("%04X: LD   V%02X [I]\n", chip->PC, regx);
    for (int i = 0; i < CHIP8E_REG_MASK(regx); i++)
        chip->V[i] = chip->memory[chip->I + i];
    chip->PC += 2;
}

#endif //__INSTRUCTIONS_H

