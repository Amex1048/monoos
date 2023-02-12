#ifndef HAL_H
#define HAL_H

#include <stddef.h>
#include <stdint.h>

#define sti() __asm__ volatile ("sti;")
#define cli() __asm__ volatile ("cli;")
#define hlt() __asm__ volatile ("hlt;") 

static inline void outb(uint16_t port, uint8_t value)
{
    __asm__ volatile("outb %0, %1" ::"a"(value), "d"(port));
}

static inline void outw(uint16_t port, uint16_t value)
{
    __asm__ volatile("outw %0, %1" ::"a"(value), "d"(port));
}

static inline void outl(uint16_t port, uint32_t value)
{
    __asm__ volatile("outl %0, %1" ::"a"(value), "d"(port));
}

static inline uint8_t inb(uint16_t port)
{
    uint8_t in;
    __asm__ volatile("inb %1, %0" : "=a"(in) : "d"(port));
    return in;
}

static inline uint16_t inw(uint16_t port)
{
    uint16_t in;
    __asm__ volatile("inw %1, %0" : "=a"(in) : "d"(port));
    return in;
}

static inline uint32_t inl(uint16_t port)
{
    uint32_t in;
    __asm__ volatile("inl %1, %0" : "=a"(in) : "d"(port));
    return in;
}

#endif /* HAL_H */