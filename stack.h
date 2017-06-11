#ifndef __STACK_H
#define __STACK_H

#include "chip8.h"

/**
 * Stack operations for CHIP8 Stack
 **/
 
 int chip8_stack_init(chip8_p chip);
 int chip8_stack_push(chip8_p chip, uint16_t n);
 int chip8_stack_pop(chip8_p chip, uint16_t *np);
 void chip8_stack_print(chip8_p chip);
 
#endif // __STACK_H

