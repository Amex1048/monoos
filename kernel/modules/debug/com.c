#include <config.h>
#ifdef DEBUG_COM

#include <stddef.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>

#include <hal.h>

#include <debug.h>
#include <devices/com.h>

#include "dbgdef.h"

#define COM1 0x3F8

#define DLLB  COM1          // Divisor Latch Low Byte 
#define IER  (COM1 + 0x1)   // Interrupt Enable Register
#define DLHB (COM1 + 0x1)   // Divisor Latch High Byte
#define IIR  (COM1 + 0x2)   // Interrupt Identification Register
#define FIFO (COM1 + 0x2)   // FIFO Control Register
#define LCR  (COM1 + 0x3)   // Line Control Register
#define MCR  (COM1 + 0x4)   // Modem Control Register
#define MSR  (COM1 + 0x5)   // Modem Status Register

static bool initialised;

static inline bool transmit_empty(void)
{
    return (inb(MSR) & 0x20) ? true : false;
}

int32_t com_send_char(char c)
{
    if(!initialised) return -1;

    while(!transmit_empty());
    outb(COM1, (uint8_t)c);

    return 1;
}

int32_t init_serial(void)
{
    outb(IER, 0x00);       // Disable interrupts
    outb(LCR, 0x80);       // Enable DLAB (set baud rate divisor)

    outb(DLLB, 0x03);      // Set divisor to 3 (lo byte) 38400 baud
    outb(DLHB, 0x00);      //                  (hi byte)

    outb(LCR, 0x03);       // 8 bits, no parity, one stop bit
    outb(FIFO, 0xC7);      // Enable FIFO, clear them, with 14-byte threshold
    outb(MCR, 0x0B);       // IRQs enabled, RTS/DSR set

    outb(MCR, 0x1E);       // Set in loopback mode to test serial port
    outb(COM1, 0xAE);      // send byte 0xAE and check if serial returns same byte

    if(inb(COM1) != 0xAE) 
    {
        initialised = false;
        return -1;  // Data is not the same as was set, COM doesn't work
    }

    outb(MCR, 0x0F); // Serial is working, set it back in non-loopback mode

    initialised = true;
    return 0;
}

#endif