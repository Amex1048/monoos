#ifndef KPRINTF_H
#define KPRINTF_H

#include <config.h>
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

int32_t vprintf(const char *format, va_list args, int32_t (put_char)(char));

int32_t kprintf(const char *format, ...);

#ifdef DEBUG
int32_t dbg_kprintf(const char *format, ...);
#endif

#endif /* KPRINTF_H */