#include <stddef.h>
#include <stdint.h>

#include <string.h>

void *memset(void *ptr, uint32_t value, size_t num)
{
	void *return_ptr = ptr;

	uint8_t mod = num % 4;
	for(uint8_t i = mod; i > 0; --i)
	{
		*(uint8_t *)(ptr) = (uint8_t)value;
		ptr = (uint8_t *)ptr + 1;
	}
	num /= 4;

	__asm__ volatile ("cld; rep stosl;"
			:
			: "a" (((uint32_t)value | (uint32_t)value << 8) | ((uint32_t)value << 16 | (uint32_t)value << 24)),
			  "D" (ptr),
			  "c" (num)
			: "cc", "memory"
	);

	return return_ptr;
}

void *memcpy(void* destination, const void* source, size_t num)
{
	void* using_source = (void*)source;
	void* return_destination = destination;

	uint8_t mod = num % 4; 
	for(uint8_t i = mod; i > 0; --i)
	{
		*(uint8_t*)(destination) = *(uint8_t*)using_source;
		using_source = (uint8_t *)using_source + 1;
		destination = (uint8_t *)destination + 1;
	}
	num /= 4;

	__asm__ volatile("cld; rep movsl;"
			:
			: "S"(using_source), 
			  "D"(destination),
			  "c"(num)
			: "cc", "memory"
	);

	return return_destination;
}

size_t strlen(const char *string)
{
	size_t size = 0;

	while(*string)
	{
		++size;
		++string;
	}

	return size;
}