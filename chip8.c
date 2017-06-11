#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#include "chip8.h"
#include "stack.h"
#include "sprites.h"
#include "instructions.h"

void chip8_init(chip8_p chip)
{
    srand(time(NULL));
    chip->state = CHIP_STATE_NORMAL;
    chip8_stack_init(chip);
    // registers
    for (int i = 0; i < 16; i++) {
        chip->V[i] = 0;
    }
    // memory
    for (int i = 0; i < CHIP8E_MEM_SIZE; i++) {
        chip->memory[i] = CHIP8_EMPTY_BYTE;
    }
    chip->cmd_delay_ts.tv_sec = CHIP8E_CMD_TS_DELAY_MS / 1000;
    chip->cmd_delay_ts.tv_nsec = (CHIP8E_CMD_TS_DELAY_MS % 1000) * 1000000;

    // Load sprites to memory
    chip8_block_to_mem(chip, CHIP8E_MEM_OFFSET_SPRITE_START, sprites, 5 * 16);
    // Start executing program memory
    chip->PC = CHIP8E_MEM_OFFSET_PROGRAM_START;
    chip->video_dirty = true;

}

void chip8_load_program_block(chip8_p chip, uint8_t *buf, uint16_t size)
{
    chip8_block_to_mem(chip, CHIP8E_MEM_OFFSET_PROGRAM_START, buf, size);
}

void chip8_block_to_mem(chip8_p chip, uint16_t offset, uint8_t *buf, uint16_t size)
{
    memcpy(chip->memory + CHIP8E_MEM_MASK(offset), buf, CHIP8E_MEM_MASK(size));
}

void chip8_mem_to_block(chip8_p chip, uint16_t offset, uint8_t *buf, uint16_t size)
{
    memcpy(buf, chip->memory + CHIP8E_MEM_MASK(offset), CHIP8E_MEM_MASK(size));
}

int chip8_file_to_block(chip8_p chip, char *filename, uint8_t *buf, uint16_t *size)
{
    struct stat sb;

    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        printf("%s\n", strerror(errno));

        return EXIT_FAILURE;
    }

    if (fstat(fd, &sb) < 0) {
        printf("%s\n", strerror(errno));

        return EXIT_FAILURE;
    }

    // Limit readable size by available emulator memory
    *size = CHIP8E_MEM_MASK(sb.st_size);
    printf("Reading %d bytes from %s\n", *size, filename);

    if (read(fd, buf, *size) < 0) {
        printf("%s\n", strerror(errno));
        close(fd);

        return EXIT_FAILURE;
    }

    close(fd);
    return EXIT_SUCCESS;
}

void chip8_trap(chip8_p chip)
{
    //chip->state = CHIP_STATE_EXCEPTION;
    // Print registers:
    printf("--------------------------------\n");
    printf("CPU State:\n");
    printf("I:%04X[%02X] PC:%04X FLAG:%02X SP:%02X\n",
        chip->I, chip->memory[chip->I], chip->PC, chip->V[VF], chip->SP);
    printf("--------------------------------\n");

    for (int i = 0; i < 4; i++)
        printf("V%02X: %02x V%02X: %02X V%02X: %02x V%02X: %02x\n",
            4 * i, chip->V[4 * i ],
            4 * i + 1, chip->V[4 * i + 1],
            4 * i + 2, chip->V[4 * i + 2],
            4 * i + 3, chip->V[4 * i + 3]);
    printf("--------------------------------\n");
    // Print stack:
    chip8_stack_print(chip);
    chip8_memdump(chip, chip->PC);
}

void chip8_memdump(chip8_p chip, uint16_t addr)
{
    printf("--------------------------------\n");
    printf("Memory Block(%d bytes):\n", CHIP8E_MEMDUMP_BLOCK_SIZE);
    printf("%04x:01 02 03 04 05 06 07 08\n", addr);
    printf("--------------------------------\n");
    for (int i = 0; i < (CHIP8E_MEMDUMP_BLOCK_SIZE >> 3); i++)
    printf("+%02X: %02x %02x %02x %02x %02x %02x %02x %02x\n",
            8 * i,
            chip->memory[addr + 8 * i ],
            chip->memory[addr + 8 * i + 1],
            chip->memory[addr + 8 * i + 2],
            chip->memory[addr + 8 * i + 3],
            chip->memory[addr + 8 * i + 4],
            chip->memory[addr + 8 * i + 5],
            chip->memory[addr + 8 * i + 6],
            chip->memory[addr + 8 * i + 7]
            );
}

#define CHIP8_INSTR_CMD(cmd) (((cmd) & 0xF000) >> 12)
#define CHIP8_INSTR_R1(cmd) (((cmd) & 0x0F00) >> 8)
#define CHIP8_INSTR_R2(cmd) (((cmd) & 0x00F0) >> 4)
#define CHIP8_INSTR_NIBBLE(cmd) (((cmd) & 0x00F))
#define CHIP8_INSTR_BYTE(cmd) (((cmd) & 0x00FF))
#define CHIP8_INSTR_ADDR(cmd) (((cmd) & 0x0FFF))

// fetch instruction
// decode
// execute
void chip8_cycle(chip8_p chip)
{
    uint16_t cmd = chip->memory[chip->PC] << 8 | chip->memory[chip->PC + 1];
    chip8_interpret_cmd(chip, cmd);
}

void chip8_interpret_cmd(chip8_p chip, uint16_t cmd)
{
    switch (CHIP8_INSTR_CMD(cmd)) {
        case 0x0:
            if (0x00E0 == cmd) {
                i_cls(chip);
            } else
            if (0x00EE == cmd) {
                i_ret(chip);
            } else {
                i_sys(chip, CHIP8_INSTR_ADDR(cmd));
            }
        break;
        case 0x1:
            i_jp(chip, CHIP8_INSTR_ADDR(cmd));
        break;
        case 0x2:
            i_call(chip, CHIP8_INSTR_ADDR(cmd));
        break;
        case 0x3:
            i_sevxb(chip, CHIP8_INSTR_R1(cmd), CHIP8_INSTR_BYTE(cmd));
        break;
        case 0x4:
            i_snevxb(chip, CHIP8_INSTR_R1(cmd), CHIP8_INSTR_BYTE(cmd));
        break;
        case 0x5:
            i_sevxvy(chip, CHIP8_INSTR_R1(cmd), CHIP8_INSTR_R2(cmd));
        break;
        case 0x6:
            i_ldvxb(chip, CHIP8_INSTR_R1(cmd), CHIP8_INSTR_BYTE(cmd));
        break;
        case 0x7:
            i_addvxb(chip, CHIP8_INSTR_R1(cmd), CHIP8_INSTR_BYTE(cmd));
        break;
        case 0x8:
            switch (cmd & 0x000F) {
                case 0x00:
                    i_ldvxvy(chip, CHIP8_INSTR_R1(cmd), CHIP8_INSTR_R2(cmd));
                break;
                case 0x01:
                    i_orvxvy(chip, CHIP8_INSTR_R1(cmd), CHIP8_INSTR_R2(cmd));
                break;
                case 0x02:
                    i_andvxvy(chip, CHIP8_INSTR_R1(cmd), CHIP8_INSTR_R2(cmd));
                break;
                case 0x03:
                    i_xorvxvy(chip, CHIP8_INSTR_R1(cmd), CHIP8_INSTR_R2(cmd));
                break;
                case 0x04:
                    i_addvxvy(chip, CHIP8_INSTR_R1(cmd), CHIP8_INSTR_R2(cmd));
                break;
                case 0x05:
                    i_subvxvy(chip, CHIP8_INSTR_R1(cmd), CHIP8_INSTR_R2(cmd));
                break;
                case 0x06:
                    i_shrvx(chip, CHIP8_INSTR_R1(cmd));
                break;
                case 0x07:
                    i_subnvxvy(chip, CHIP8_INSTR_R1(cmd), CHIP8_INSTR_R2(cmd));
                break;
                case 0x0E:
                    i_shlvx(chip, CHIP8_INSTR_R1(cmd));
                break;
                default:
                    chip->state = CHIP_STATE_EXCEPTION;
                    // TODO die on error: unknown opcode
                break;
            }
        break;
        case 0x9:
            i_snevxvy(chip, CHIP8_INSTR_R1(cmd), CHIP8_INSTR_R2(cmd));
        break;
        case 0xA:
            i_ldiw(chip, CHIP8_INSTR_ADDR(cmd));
        break;
        case 0xB:
            i_jpv0w(chip, CHIP8_INSTR_ADDR(cmd));
        break;
        case 0xC:
            i_rndvxb(chip, CHIP8_INSTR_R1(cmd), CHIP8_INSTR_BYTE(cmd));
        break;
        case 0xD:
            i_drwvxvyn(chip, CHIP8_INSTR_R1(cmd), CHIP8_INSTR_R2(cmd), CHIP8_INSTR_NIBBLE(cmd));
        break;
        case 0xE:
            switch (CHIP8_INSTR_BYTE(cmd)) {
                case 0x9E:
                    i_skpvx(chip, CHIP8_INSTR_R1(cmd));
                break;
                case 0xA1:
                    i_sknpvx(chip, CHIP8_INSTR_R1(cmd));
                break;
                default:
                    chip->state = CHIP_STATE_EXCEPTION;
                    // TODO die on error: unknown opcode
                break;
            }
        break;
        case 0xF:
            switch (CHIP8_INSTR_BYTE(cmd)) {
                case 0x07:
                    i_ldvxdt(chip, CHIP8_INSTR_R1(cmd));
                break;
                case 0x0A:
                    i_ldvxk(chip, CHIP8_INSTR_R1(cmd));
                break;
                case 0x15:
                    i_lddtvx(chip, CHIP8_INSTR_R1(cmd));
                break;
                case 0x18:
                    i_ldstvx(chip, CHIP8_INSTR_R1(cmd));
                break;
                case 0x1E:
                    i_addivx(chip, CHIP8_INSTR_R1(cmd));
                break;
                case 0x29:
                    i_ldfvx(chip, CHIP8_INSTR_R1(cmd));
                break;
                case 0x33:
                    i_ldbvx(chip, CHIP8_INSTR_R1(cmd));
                break;
                case 0x55:
                    i_ldivx(chip, CHIP8_INSTR_R1(cmd));
                break;
                case 0x65:
                    i_ldvxi(chip, CHIP8_INSTR_R1(cmd));
                break;
                default:
                    chip->state = CHIP_STATE_EXCEPTION;
                    // TODO die on error: unknown opcode
                break;
            }
        break;
        default:
            chip->state = CHIP_STATE_EXCEPTION;
            // not possible, exhausted cmd byte values
        break;
    }
}

