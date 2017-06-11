#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "stack.h"

int chip8_stack_init(chip8_p chip) 
{
	chip->SP = 0;
	for (int i = 0; i < CHIP8E_STACK_SIZE; i++) {
		chip->stack[i] = CHIP8_EMPTY_WORD;
	}
	return EXIT_SUCCESS;
}
int chip8_stack_push(chip8_p chip, uint16_t n)
{
	if (chip->SP == CHIP8E_STACK_SIZE) {
		printf("Stack full!\n");
		return EXIT_FAILURE;
	}
	// printf("Push:[%d]=%d\n", chip->SP, n);	
	chip->stack[chip->SP] = n;
	chip->SP++;
	return EXIT_SUCCESS;
}

int chip8_stack_pop(chip8_p chip, uint16_t *np)
{
	if (chip->SP == 0) {
		printf("Stack empty!\n");
		return EXIT_FAILURE;
	}
	
	chip->SP--;	
	*np = chip->stack[chip->SP];
	// printf("Pop:[%d]=%d\n", chip->SP, *np);		

	return EXIT_SUCCESS;
}

void chip8_stack_print(chip8_p chip)
{	
	printf("Stack:\n");	
	for (int i = 0; i < CHIP8E_STACK_SIZE; i++)
		printf("[%02X] = %04x%s\n", 
			i, chip->stack[i], (chip->SP == i) ? " <- SP" : "");
}
