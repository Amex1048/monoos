#ifndef STRING_H
#define STRING_H

#include <stddef.h>
#include <stdint.h>

void *memset(void *ptr, uint32_t value, size_t num);
void *memcpy(void *destination, const void *source, size_t num);

size_t strlen(const char *string);

#endif /* STRING_H */