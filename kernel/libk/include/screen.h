#ifndef SCREEN_H
#define SCREEN_H

#include <types.h>

#define DEFAULT_COLOR 0x3F

/* Hardware text mode color constants. */
enum vga_color {
	VGA_COLOR_BLACK = 0,
	VGA_COLOR_BLUE = 1,
	VGA_COLOR_GREEN = 2,
	VGA_COLOR_CYAN = 3,
	VGA_COLOR_RED = 4,
	VGA_COLOR_MAGENTA = 5,
	VGA_COLOR_BROWN = 6,
	VGA_COLOR_LIGHT_GREY = 7,
	VGA_COLOR_DARK_GREY = 8,
	VGA_COLOR_LIGHT_BLUE = 9,
	VGA_COLOR_LIGHT_GREEN = 10,
	VGA_COLOR_LIGHT_CYAN = 11,
	VGA_COLOR_LIGHT_RED = 12,
	VGA_COLOR_LIGHT_MAGENTA = 13,
	VGA_COLOR_LIGHT_BROWN = 14,
	VGA_COLOR_WHITE = 15,
};

// start address of vidmem is  0xB8000
// final address of vidmem is (0xB8FA0 - 1) if address >= 0xB8FA0 it is not video memory territory
// size of address of vidmem is 4000 decimal or FA0 hex

extern uint16_t cursorX, cursorY;
extern uint8_t color;

extern uint8_t canOutput;

extern const uint8_t SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_DEPTH;		// screen width, height and depth
extern const uint8_t TAB_SIZE;

void ClearLine(uint8_t from, uint8_t to);

void UpdateCursor();

void SetCursor(uint16_t x, uint16_t y);

void SetPointer(uint16_t x, uint16_t y);

void ClearScreen();

void ScrollUp(uint8_t lineNumber);

void NewLineCheck();

void Printch(char c);

void Print(char* str);

void SetColor(uint8_t background, uint8_t foreground);

void SetForegroundColor(uint8_t _color);

void SetBackgroundColor(uint8_t _color);

uint8_t MakeColor(uint8_t background, uint8_t foreground);

void PrintLine();

#endif