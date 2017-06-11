#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "chip8.h"
#include "sprites.h"
#include "stack.h"

void usage()
{
    // TODO
    printf("Usage: chip8e -p progname -n\n");
    printf("Options:\n"
    "\t-p file - specifies the binary to be loaded.\n"
    "\t-n      - disables sound.\n"
    "\t-h      - this help.\n");
}

int execute_binary(char *binary, bool sound_flag)
{
    chip8_t chip;
    chip8_init(&chip);

    // Load program code to emulator memory
    uint8_t file_buf[CHIP8E_MEM_SIZE + 1];
    uint16_t size = 0;
    memset(file_buf, 0x00, CHIP8E_MEM_SIZE);
    if (EXIT_SUCCESS != chip8_file_to_block(&chip, binary, file_buf, &size)) {
        printf("Error loading test file %s.\n", binary);

        exit(EXIT_FAILURE);
    }
    chip8_block_to_mem(&chip, CHIP8E_MEM_OFFSET_PROGRAM_START, file_buf, size);
    // add exit handler

    // Start executing program code
    while (chip.state == CHIP_STATE_NORMAL) {
        chip8_cycle(&chip);
        if (chip.DT > 0)
            chip.DT --;

        if (chip.ST > 0) {
            if (1 == chip.ST) {
            // TODO beep
            printf("Beep!\n");
            }
            chip.ST --;
        }

        if (chip.video_dirty) {
            //printf("Redraw required\n");
            // TODO redraw
            chip.video_dirty = false;
        }
        if (nanosleep(&(chip.cmd_delay_ts), NULL))
            printf("Sleep interrupted.");
    }

    if (chip.state == CHIP_STATE_EXCEPTION) {
        chip8_trap(&chip);
    }

    return(EXIT_SUCCESS);
}

int main(int argc, char*argv[])
{
    printf("CHIP8 Emulator\n");
    char *binary = NULL;

    if (argc < 2) {
        usage();
        exit(EXIT_SUCCESS);
    }

    bool sound_flag = 1;
    int ch;
    while ((ch = getopt(argc, argv, "p:nh")) != -1) {
        switch (ch) {
            case 'p':
                binary = strdup(optarg);
                if (NULL == binary) {
                    printf("Out of memory: %s\n", strerror(errno));
                    exit(EXIT_FAILURE);
                }
            break;
            case 'n':
                sound_flag = 0;
                printf("Sound disabled.\n");
            break;
            case 'h':
            case '?':
            default:
                usage();
                exit(EXIT_SUCCESS);
            break;
        }
    }

    if (NULL != binary) {
        int result = execute_binary(binary, sound_flag);
        free(binary);
        return result;
    } else {
        usage();
        exit(EXIT_SUCCESS);
    }
}
