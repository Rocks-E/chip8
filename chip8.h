#pragma once

#include "display.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>


#define MEMORY_SIZE 0x1000
#define REGISTERS_SIZE 0x10
#define CHAR_SIZE 0x5
#define FONT_SIZE (CHAR_SIZE * 0x10)

/*/
 * Everything is held in memory in this implementation.
 * Early interpreters had all of their own code in the first 512 bytes, 
 * but we don't need that space so we might as well make use of it.
 * It's common for font data to be stored here but that only needs so much space.
 * Why not put everything else in there too?
 * Let's see if we can do as much processing as possible using just unused memory cells...
 *
 * Memory mapping:
 * 	000:001 -> Program counter (12 bits, 0 -> 1.5)
 *	001:002 -> Index register (12 bits, 1.5 -> 3)
 *	003:003 -> Stack counter
 *	004:004 -> Delay timer
 *	005:005 -> Sound timer
 *	006:007 -> KB_START
 *	008:00F -> Display pointer
 *	010:01F -> Registers
 *	020:04F -> Variable space
 *	050:0FF -> Font data
 *	100:1FF -> Stack data
/*/
#define PC_START	0x000
#define IR_START	0x001
#define SC_START	0x003
#define DT_START	0x004
#define ST_START	0x005
#define KB_START	0x006
#define DISP_START	0x008
#define REG_START	0x010
#define VAR_START	0x020
#define FONT_START	0x050
#define STACK_START	0x1FF
#define PROG_START	0x200

#define INCREMENT_PC(chip) {chip->memory[PC_START + 1] += 0x20; if(!(chip->memory[PC_START + 1] & 0xF0)) chip->memory[PC_START]++;}

#define PC(chip) ((chip->memory[PC_START] << 4) | (chip->memory[PC_START + 1] >> 4))
#define SET_PC(chip, addr) {chip->memory[PC_START] = addr >> 4; chip->memory[PC_START + 1] = (chip->memory[PC_START + 1] & 0x0F) | (addr << 4);}
#define PCX(chip, offset) (chip->memory[PC(chip) + offset])

#define IR(chip) (((chip->memory[IR_START] & 0xF) << 8) | (chip->memory[IR_START + 1]))
#define SET_IR(chip, addr) {chip->memory[IR_START] = (chip->memory[IR_START] & 0xF0) | (addr >> 4); chip->memory[IR_START + 1] = addr;}
#define IRX(chip, offset) (chip->memory[IR(chip) + offset])

#define STACK_COUNTER(chip) (chip->memory[SC_START])
#define STACKX(chip, offset) (chip->memory[STACK_START - chip->memory[STACK_COUNTER(chip)] + offset])
#define STACK_ADDR(chip) ((STACKX(chip, 0) << 4) | STACKX(chip, 1))

#define DELAY_TIMER(chip) (chip->memory[DT_START])
#define SOUND_TIMER(chip) (chip->memory[ST_START])
#define KEYBOARD(chip) (*(uint16_t *)&chip->memory[KB_START])
#define DISPLAY_ADDR(chip) (*(uint64_t *)&chip->memory[DISP_START])
#define DISPLAY(chip) ((display_t *)DISPLAY_ADDR(chip))

#define V0(chip) (chip->memory[REG_START])
#define VX(chip) (chip->memory[REG_START + X(chip)])
#define VY(chip) (chip->memory[REG_START + Y(chip)])
#define VF(chip) (chip->memory[REG_START + 0xF])

#define FONT(chip, char) (chip->memory[FONT_START + char * CHAR_SIZE])

#define VAR(chip, byte, type) (*(type *)&chip->memory[VAR_START + byte])

#define OPCODE(chip) (VAR(chip, 2, uint8_t))
#define X(chip) (VAR(chip, 3, uint8_t))
#define Y(chip) (VAR(chip, 4, uint8_t))
#define N(chip) (VAR(chip, 5, uint8_t))
#define NN(chip) (VAR(chip, 6, uint8_t))
// This will overwrite our instruction but that's fine since we already have the opcode stored by the time we set NNN
#define NNN(chip) (VAR(chip, 0, uint16_t))
#define TEMP_BYTE(chip) (VAR(chip, 9, uint8_t))
#define TEMP_SHORT(chip) (VAR(chip, 10, uint16_t))

/* Why not save on memory just that little bit more by just incorporating this directly into the init function?
const uint8_t font_data[FONT_SIZE] = {
	0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
	0x20, 0x60, 0x20, 0x20, 0x70, // 1
	0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
	0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
	0x90, 0x90, 0xF0, 0x10, 0x10, // 4
	0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
	0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
	0xF0, 0x10, 0x20, 0x40, 0x40, // 7
	0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
	0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
	0xF0, 0x90, 0xF0, 0x90, 0x90, // A
	0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
	0xF0, 0x80, 0x80, 0x80, 0xF0, // C
	0xE0, 0x90, 0x90, 0x90, 0xE0, // D
	0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
	0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};
*/

typedef struct chip8_d {
	uint8_t memory[MEMORY_SIZE];
} chip8_t;

chip8_t *chip8_new();
void chip8_init(chip8_t *chip);
void chip8_load(chip8_t *chip, char *filename);
uint8_t chip8_cycle(chip8_t *chip);
void chip8_attach_display(chip8_t *chip, display_t *display);