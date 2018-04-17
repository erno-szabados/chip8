#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <SDL.h>

#include "chip8.h"
#include "sprites.h"
#include "stack.h"

void redraw(SDL_Renderer *renderer, uint8_t *video_buffer) {
    SDL_Rect rect = {
    .x = 0,
    .y = 0,
    .w = 8,
    .h = 8
    };

    for (int i = 0; i < CHIP8E_XRES; i++)
        for (int j = 0; j < CHIP8E_YRES; j++) {
        if (0x01 == video_buffer[i * CHIP8E_XRES + j]) {
            SDL_SetRenderDrawColor(renderer, 0x40, 0x40, 0x40, 0xFF);
        } else {
            SDL_SetRenderDrawColor(renderer, 0x80, 0x80, 0x80, 0xFF);
        }
        rect.x = i * (rect.w + 2);
        rect.y = j * (rect.h + 2);
        SDL_RenderFillRect(renderer, &rect);
    }
    SDL_RenderPresent(renderer);
}

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

  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("SDL Initialization failed:%s.\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    SDL_Window *window = SDL_CreateWindow("Hello, World!",
        100, 100, 640, 320, SDL_WINDOW_SHOWN);
    if (window == NULL) {
            printf("SDL Window creation failed: %s.\n",  SDL_GetError());
            SDL_Quit();
            exit(EXIT_FAILURE);
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC);
    if (renderer == NULL) {
            printf("SDL Renderer creation failed: %s.\n",  SDL_GetError());
            SDL_Quit();
            exit(EXIT_FAILURE);
    }

    chip8_block_to_mem(&chip, CHIP8E_MEM_OFFSET_PROGRAM_START, file_buf, size);

    // Start executing program code
    while (chip.state == CHIP_STATE_NORMAL) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    printf("Quit event.\n");
                    chip.state = CHIP_STATE_EXIT;
                    SDL_FlushEvents(SDL_FIRSTEVENT, SDL_LASTEVENT);
                break;
                case SDL_KEYDOWN:
                case SDL_KEYUP:
                    printf("Key down event: %x %x.\n", event.key.keysym.sym, SDLK_ESCAPE);
                    switch (event.key.keysym.sym) {
                        case SDLK_ESCAPE:
                            chip.state = CHIP_STATE_EXIT;
                        break;
                    }
                SDL_FlushEvents(SDL_FIRSTEVENT, SDL_LASTEVENT);
                break;
                default:
                    SDL_FlushEvents(SDL_FIRSTEVENT, SDL_LASTEVENT);
                break;
            }
            SDL_RenderPresent(renderer);

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
                redraw(renderer, chip.video_buffer);
                chip.video_dirty = false;
            }

            if (nanosleep(&(chip.cmd_delay_ts), NULL))
                printf("Sleep interrupted.");
        }
    }

    if (chip.state == CHIP_STATE_EXCEPTION) {
        chip8_trap(&chip);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

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
