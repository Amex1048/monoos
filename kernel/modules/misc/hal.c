#include <stddef.h>
#include <stdint.h>

//#include <hal.h>

inline void outb(uint16_t port, uint8_t value)
{
    __asm__ volatile("outb %0, %1" ::"a"(value), "d"(port));
}

inline void outw(uint16_t port, uint16_t value)
{
    __asm__ volatile("outw %0, %1" ::"a"(value), "d"(port));
}

inline void outl(uint16_t port, uint32_t value)
{
    __asm__ volatile("outl %0, %1" ::"a"(value), "d"(port));
}

inline uint8_t inb(uint16_t port)
{
    uint8_t in;
    __asm__ volatile("inb %1, %0" : "=a"(in) : "d"(port));
    return in;
}

inline uint16_t inw(uint16_t port)
{
    uint16_t in;
    __asm__ volatile("inw %1, %0" : "=a"(in) : "d"(port));
    return in;
}

inline uint32_t inl(uint16_t port)
{
    uint32_t in;
    __asm__ volatile("inl %1, %0" : "=a"(in) : "d"(port));
    return in;
}