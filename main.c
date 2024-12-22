#include "chip8.h"
#include "display.h"

int32_t main() {
	
	chip8_t *chip = chip8_new();
	chip8_load(chip, "IBM_Logo.ch8");
	
	return 0;
	
}