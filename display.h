#pragma once


#include <inttypes.h>

#define DISPLAY_WIDTH 64
#define DISPLAY_HEIGHT 32

typedef struct display_d {
	// Screen data, each index is one line of pixels
	uint64_t screen[DISPLAY_HEIGHT];
	// Need a Windows native window here? Probably still try to use the screen array for holding what we want to draw
} display_t;

void display_init(display_t *display);
void display_clear_screen(display_t *display);
// Return 1 if the pixel was flipped off, 0 otherwise
uint8_t display_flip_pixel(display_t *display, uint8_t x, uint8_t y);