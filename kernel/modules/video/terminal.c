#include <stddef.h>
#include <stdint.h>

#include <hal.h>
#include <string.h>
#include <memory.h>

#include <devices/terminal.h>

#define CURSOR_START_REG 14
#define CURSOR_END_REG 15

#define BDA_REGION 0
#define BDA_SIZE 512
#define BDA_SCREEN_WIDTH 0x44A
#define BDA_CONTROL_PORT 0x463

#define IO_REGION 0xB8000
#define REGION_SIZE (32 * 1024)

typedef struct screen_char
{
	int8_t character;
	uint8_t attributes;
} __attribute__((packed)) screen_char;

screen_char *video_buffer;
uint16_t screen_width;
uint16_t screen_height;
uint16_t control_port;

uint16_t cursor;
uint8_t attributes;

void move_cursor(uint16_t position)
{
	cursor = position;

	if (position >= screen_width * screen_height)
	{
		screen_char *dest_pos = video_buffer;
		screen_char *src_pos = video_buffer + screen_width;

		for (uint16_t i = 0; i < screen_height - 1; ++i)
		{
			memcpy(dest_pos, src_pos, screen_width * 2);

			dest_pos += screen_width;
			src_pos += screen_width;
		}

		memset(dest_pos, 0, screen_width * 2);

		cursor = (uint16_t)(screen_width * (screen_height - 1));
	}

	outb(control_port, CURSOR_START_REG);
	outb(control_port + 1, (uint8_t)(cursor >> 8));
	outb(control_port, CURSOR_END_REG);
	outb(control_port + 1, (uint8_t)cursor);
}

int32_t print_char(char character)
{
	switch (character)
	{
	case '\n':
		move_cursor((uint16_t)((cursor / screen_width + 1) * screen_width));
		break;
	default:
		video_buffer[cursor].character = character;
		video_buffer[cursor].attributes = attributes;
		move_cursor(++cursor);
	}

	return 1;
}

int32_t print_string(const char *string)
{
	int printed = 0;
	while (*string)
	{
		print_char(*string);
		++string;
		++printed;
	}

	return printed;
}

void print_byte(uint8_t number)
{
	uint8_t h_character = number >> 4;
	uint8_t l_character = number & 0x0F;

	if (h_character < 10)
	{
		print_char((char)(48 + h_character));
	}
	else
	{
		print_char((char)(65 + h_character - 10));
	}

	if (l_character < 10)
	{
		print_char((char)(48 + l_character));
	}
	else
	{
		print_char((char)(65 + l_character - 10));
	}
}

inline void print_word(uint16_t number)
{
	print_byte((uint8_t)(number >> 8));
	print_byte((uint8_t)number);
}

inline void print_dword(uint32_t number)
{
	print_word((uint16_t)(number >> 16));
	print_word((uint16_t)number);
}

void print_memory(const void *src, uint32_t count)
{
	for (size_t i = 0; i < count; ++i)
	{
		print_byte(*(uint8_t *)src);
		src = (uint8_t *)src + 1;
		print_string("  ");
	}

	print_char('\n');
}

void set_color(enum color forecolor, enum color backcolor)
{
	attributes = (uint8_t)((backcolor << 4) | (forecolor & 0x0F));
}

void init_terminal()
{
	video_buffer = map_io((void *)IO_REGION, REGION_SIZE);
	void *bda = map_io(BDA_REGION, BDA_SIZE);

	screen_width = *(uint16_t *)((uint8_t *)bda + BDA_SCREEN_WIDTH);
	screen_height = 25;

	control_port = *(uint16_t *)((uint8_t *)bda + BDA_CONTROL_PORT);

	uint8_t h_cursor, l_cursor;
	outb(control_port, CURSOR_START_REG);
	h_cursor = inb(control_port + 1);
	outb(control_port, CURSOR_END_REG);
	l_cursor = inb(control_port + 1);
	cursor = (uint16_t)(h_cursor << 8) + l_cursor;

	unmap_io(bda);

	set_color(LIGHT_GREY, BLACK);
}