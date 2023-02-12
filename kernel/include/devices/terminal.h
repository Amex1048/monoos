#ifndef TERMINAL_H
#define TERMINAL_H

#include <stddef.h>
#include <stdint.h>

enum color
{
	BLACK = 0,
	BLUE = 1,
	GREEN = 2,
	CYAN = 3,
	RED = 4,
	MAGENTA = 5,
	BROWN = 6,
	LIGHT_GREY = 7,
	DARK_GREY = 8,
	LIGHT_BLUE = 9,
	LIGHT_GREEN = 10,
	LIGHT_CYAN = 11,
	LIGHT_RED = 12,
	LIGHT_MAGENTA = 13,
	YELLOW = 14,
	WHITE = 15
};

int32_t print_char(char c);
int32_t print_string(const char *s);

void print_byte(uint8_t number);
void print_word(uint16_t number);
void print_dword(uint32_t number);

void print_memory(const void *src, uint32_t count);
void set_color(enum color forecolor, enum color backcolor);
void move_cursor(uint16_t position);

void init_terminal(void);

#endif /* TERMINAL_H */