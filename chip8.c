#include "chip8.h"

void chip8_jump(chip8_t *chip, uint16_t addr);
void chip8_call(chip8_t *chip, uint16_t addr);
void chip8_return(chip8_t *chip);

chip8_t *chip8_new() {
	
	chip8_t *chip = (chip8_t *)malloc(sizeof(chip8_t));
	
	chip8_init(chip);
	
	return chip;
	
}

void chip8_init(chip8_t *chip) {
	
	// Font data in scope here so it isn't kept allocated for the life of the program
	uint8_t font_data[FONT_SIZE] = {
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
	
	// Initialize everything to 0
	memset(chip->memory, 0, sizeof(chip8_t));
	
	// Copy the font data
	memcpy(chip->memory + FONT_START, font_data, FONT_SIZE);
	
}

void chip8_load(chip8_t *chip, char *filename) {
	
	FILE *fp = fopen(filename, "r");
	
	fread(chip->memory + PROG_START, sizeof(uint8_t), MEMORY_SIZE - PROG_START, fp);
	
	fclose(fp);
	
}

uint8_t chip8_cycle(chip8_t *chip) {
	
	// Fetch
	// Put the full 2-byte instruction into our temporary variable portion of memory
	VAR(chip, 0, uint8_t) = PCX(chip, 0);
	VAR(chip, 1, uint8_t) = PCX(chip, 1);
	
	INCREMENT_PC(chip);
	
	// Decode
	// Set these variables for convenience
	OPCODE(chip) = VAR(chip, 0, uint8_t) >> 4;
	X(chip) = VAR(chip, 0, uint8_t) & 0xF;
	Y(chip) = VAR(chip, 1, uint8_t) >> 4;
	N(chip) = VAR(chip, 1, uint8_t) & 0xF;
	NN(chip) = VAR(chip, 1, uint8_t);
	// This step overwrites VAR_START data but we've already stored everything else we need
	NNN(chip) = ((VAR(chip, 0, uint8_t) & 0xF) << 8) | VAR(chip, 1, uint8_t);
	
	// Execute
	switch(OPCODE(chip)) {
		
		// Special
		case 0x0:
		
			switch(NNN(chip)) {
				
				// Clear screen
				case 0x0E0:
					display_clear_screen(DISPLAY(chip));
					break;
				
				// Return from subroutine
				case 0x0EE:
					chip8_return(chip);
					break;
				
				// ERR: Used for executing processor instructions instead of interpreter instructions
				// This will be unsupported
				default:
					return 1;
				
			}
		
			break;
		
		// Unconditional jump
		case 0x1:
			chip8_jump(chip, NNN(chip));
			break;
		
		// Call subroutine
		case 0x2:
			chip8_call(chip, NNN(chip));
			break;
		
		// Skip next instruction if register X is equal to NN
		case 0x3:
			if(VX(chip) == NN(chip))
				INCREMENT_PC(chip);
			break;
		
		// Skip next instruction if register X is not equal to NN
		case 0x4:
			if(VX(chip) != NN(chip))
				INCREMENT_PC(chip);
			break;
		
		// Skip next instruction if register X is equal to register Y
		case 0x5:
			if(VX(chip) == VY(chip))
				INCREMENT_PC(chip);
			break;
		
		// Load immediate NN into register X
		case 0x6:
			VX(chip) = NN(chip);
			break;
		
		// Add immediate NN into register X
		case 0x7:
			VX(chip) += NN(chip);
			break;
		
		// ALU instructions
		case 0x8:
		
			switch(N(chip)) {
				
				// Load register X with register Y
				case 0x0:
					VX(chip) = VY(chip);
					break;
				
				// OR register X with register Y, store in register X
				case 0x1:
					VX(chip) |= VY(chip);
					break;
				
				// AND register X with register Y, store in register X
				case 0x2:
					VX(chip) &= VY(chip);
					break;
				
				// XOR register X with register Y, store in register X
				case 0x3:
					VX(chip) ^= VY(chip);
					break;
				
				// Add register X with register Y, store in register X, set VF if VX + VY > 0xFF and clear otherwise
				case 0x4:
					VF(chip) = ((TEMP_SHORT(chip) = VX(chip) + VY(chip)) > 0xFF);
					VX(chip) = TEMP_SHORT(chip);
					break;
				
				// Subtract register X from register Y, store in register X, clear VF if VX - VY is negative and set otherwise
				case 0x5:
					VF(chip) = VX(chip) < VY(chip);
					VX(chip) -= VY(chip);
					break;
				
				// Shift register X right, put the LSB into VF
				case 0x6:
					VF(chip) = VX(chip) & 1;
					VX(chip) >>= 1;
					break;
				
				// Subtract register Y from register X, store in register X, clear VF if VY - VX is negative and set otherwise
				case 0x7:
					VF(chip) = VX(chip) > VY(chip);
					VX(chip) = VY(chip) - VX(chip);
					break;
				
				// Shift register X left, put MSB into VF
				case 0xE:
					VF(chip) = VX(chip) >> 7;
					VX(chip) <<= 1;
					break;
				
				default:
					return 1;
				
				
			}
		
			break;
		
		// Skip next instruction if register X is not equal to register Y
		case 0x9:
			if(VX(chip) != VY(chip))
				INCREMENT_PC(chip);
			break;
		
		// Set index register to NNN
		case 0xA:
			SET_IR(chip, NNN(chip));
			break;
		
		// Unconditional jump with offset register 0
		case 0xB:
			chip8_jump(chip, NNN(chip) + V0(chip));
			break;
		
		// Set register X to random byte && NN
		case 0xC:
			VX(chip) = rand() && NN(chip);
			break;
		
		// Draw sprite
		case 0xD:
		
			X(chip) = VX(chip) % DISPLAY_WIDTH;
			Y(chip) = VY(chip) % DISPLAY_HEIGHT;
			
			VF(chip) = 0;
		
			for(NN(chip) = 0; NN(chip) < N(chip); NN(chip)++) {
				
				// Pull the next byte from the index register
				TEMP_BYTE(chip) = IRX(chip, NN(chip));

				for(OPCODE(chip) = 0; OPCODE(chip) < 8; OPCODE(chip)++) {
					if(TEMP_BYTE(chip) & 1)
						if(display_flip_pixel(DISPLAY(chip), X(chip) + OPCODE(chip), Y(chip) + NN(chip)))
							VF(chip) = 1;
				
			}
			
			break;
		
		// Keyboard instructions
		case 0xE:
		
			switch(NN(chip)) {
				
				// Skip next instruction if the key in register X is pressed
				case 0x9E:
					if((1 << VX(chip)) & KEYBOARD(chip))
						INCREMENT_PC(chip);
					break;
					
				// Skip next instruction if the key in register X is not pressed
				case 0xA1:
					if(!((1 << VX(chip)) & KEYBOARD(chip)))
						INCREMENT_PC(chip);
					break;
					
				default:
					return 1;
				
			}
		
			break;
		
		// Special
		case 0xF:
		
			switch(NN(chip)) {
				
				// Load register X with delay timer
				case 0x07:
					VX(chip) = DELAY_TIMER(chip);
					break;
				
				// Wait for a key press, store value of key in register X
				case 0x0A:
					//?
					while(!KEYBOARD(chip));
					VX(chip) = KEYBOARD(chip);
					break;
				
				// Set delay timer to register X
				case 0x15:
					DELAY_TIMER(chip) = VX(chip);
					break;
				
				// Set sound timer to register X
				case 0x18:
					SOUND_TIMER(chip) = VX(chip);
					break;
				
				// Add register X to index register
				case 0x1E:
					SET_IR(chip, IR(chip) + VX(chip));
					break;
				
				// Set I to the location of the font character specified in register X (only 0-F are valid)
				case 0x29:
					SET_IR(chip, FONT(chip, VX(chip)));
					break;
				
				// Store the BCD representation of register X at memory[index], memory[index + 1], memory[index + 2]
				case 0x33:
					Y(chip) = VX(chip);
					IRX(chip, 2) = Y(chip) % 10; 
					Y(chip) /= 10;
					IRX(chip, 1) = Y(chip) % 10;
					Y(chip) /= 10;
					IRX(chip, 0) = Y(chip);
					break;
				
				// Load registers 0 through register X with values from memory[index] to memory[index + X]
				case 0x55:
					for(Y(chip) = 0; Y(chip) < X(chip); Y(chip)++)
						VY(chip) = IRX(chip, Y(chip));
					break;
				
				// Store registers 0 through register X to memory from memory[index] to memory[index + X]
				case 0x65:
					for(Y(chip) = 0; Y(chip) < X(chip); Y(chip)++)
						IRX(chip, Y(chip)) = VY(chip);
					break;
				
				default:
					return 1;
				
			}
		
			break;
			
		default:
			// Unreachable code path
			fprintf(stderr, "HUH?\n");
			exit(-1);
		
		}
		
	}
	
	return 0;
	
}

// Attach the display that the clear screen and draw commands will update
void chip8_attach_display(chip8_t *chip, display_t *display) {
	DISPLAY_ADDR(chip) = (uint64_t)display;	
}

void chip8_jump(chip8_t *chip, uint16_t addr) {
	SET_PC(chip, addr);
}

void chip8_call(chip8_t *chip, uint16_t addr) {
	
	// Push to the stack
	
	STACKX(chip, 0) = PCX(chip, 0);
	STACKX(chip, 1) = PCX(chip, 1) >> 4;
	
	// Increment the stack counter
	STACK_COUNTER(chip) += 2;
	
	// Set the program counter to the jump point
	chip8_jump(chip, addr);
	
}

void chip8_return(chip8_t *chip) {
	
	// Decrement the stack counter
	STACK_COUNTER(chip) -= 2;
	
	// Jump to the address from the stack head
	chip8_jump(chip, STACK_ADDR(chip));
	
}