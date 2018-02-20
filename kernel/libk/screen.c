#include "include/screen.h"

uint16_t cursorX = 0;
uint16_t cursorY = 0;
uint8_t color = DEFAULT_COLOR;

const uint8_t SCREEN_WIDTH = 80, SCREEN_HEIGHT = 25, SCREEN_DEPTH = 2;
const uint8_t TAB_SIZE = 20;

void ClearLine(uint8_t from, uint8_t to)
{
	uint16_t row = SCREEN_WIDTH * from * SCREEN_DEPTH;
	char* vidmem = (char*)0x0B8000;

	uint16_t i = row;
	for (; i < SCREEN_WIDTH * (to + 1) * SCREEN_DEPTH; i += 2)
	{
		vidmem[i] = 0x0;
		vidmem[i + 1] = color;					// if we set color byte to zero then we get a black cursor on a black background. (we see nothing)
	}
}

void UpdateCursor()
{
	/* CRT address register is 0x3D4 */
	/* CRT data    register is 0x3D5 */

	uint16_t temp = SCREEN_WIDTH * cursorY + cursorX;

	outportb(0x3D4, 14);
	outportb(0x3D5, temp >> 8);	// CRT register send high byte location of the cursor
	outportb(0x3D4, 15);
	outportb(0x3D5, temp);		// then send the low byte location of the cursor
}

void SetCursor(uint16_t x, uint16_t y)
{
	cursorX = x;
	cursorY = y;
	UpdateCursor();
}

void SetPointer(uint16_t x, uint16_t y)
{
	cursorX = x;
	cursorY = y;
}

void ClearScreen()
{
	ClearLine(0, SCREEN_HEIGHT - 1);

	cursorX = cursorY = 0;
	UpdateCursor();
}

void ScrollUp(uint8_t lineNumber)
{
	char* vidmem = (char*)0x0B8000;

	int i = 0;
	for (; i < SCREEN_WIDTH * (SCREEN_HEIGHT - 1) * SCREEN_DEPTH; i++)
		vidmem[i] = vidmem[i + SCREEN_WIDTH * SCREEN_DEPTH * lineNumber];

	ClearLine(SCREEN_HEIGHT - 1 - lineNumber, SCREEN_HEIGHT - 1);

	if (cursorY < lineNumber)
		cursorX = cursorY = 0;
	else
		cursorY -= lineNumber;

	UpdateCursor();
}

void NewLineCheck()
{
	if (cursorY > SCREEN_HEIGHT - 1)
		ClearScreen();
}

void Printch(char c)
{
	char* vidmem = (char*)0xB8000;

	switch (c)
	{
	case 0x08:		// backspace
		cursorX--;
		vidmem[(cursorY * SCREEN_WIDTH + cursorX) * SCREEN_DEPTH] = 0x0;
		break;
	case 0x09:
		cursorX += TAB_SIZE - cursorX % TAB_SIZE;
		break;
	case '\r':
		//cursorX = 0;
		//break;
	case '\n':
		cursorX = 0;
		cursorY++;
		break;
	default:
		vidmem[(cursorY * SCREEN_WIDTH + cursorX) * SCREEN_DEPTH] = c;
		vidmem[(cursorY * SCREEN_WIDTH + cursorX) * SCREEN_DEPTH + 1] = color;
		cursorX++;
		break;
	}

	if (cursorX >= SCREEN_WIDTH)
	{
		cursorX = 0;
		cursorY++;
	}
	NewLineCheck();
	UpdateCursor();
}

void Print(char* str)
{
	while (*str != 0)
		Printch(*str++);
}

void SetColor(uint8_t background, uint8_t foreground)		// color goes: upper nibble for background, lower nibble for foreground
{
	color = (background << 4) | foreground;
}

void SetForegroundColor(uint8_t _color)
{
	color &= 0xF0;
	color |= _color;
}

void SetBackgroundColor(uint8_t _color)
{
	color &= 0x0F;
	color |= (_color << 4);
}

uint8_t MakeColor(uint8_t background, uint8_t foreground)
{
	return ((background << 4) | foreground);
}

void PrintLine()
{
	Printch('\n');
}