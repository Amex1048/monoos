#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include <stddef.h>
#include <stdint.h>

#define MPIC_COMMAND 0x20
#define MPIC_DATA 0x21
#define SPIC_COMMAND 0xA0
#define SPIC_DATA 0xA1

#define MPIC_OFFSET 0x20
#define SPIC_OFFSET 0x28

#define EOI	0x20

typedef struct int_registers
{
    uint32_t gs, fs, es, ds;
    uint32_t edi, esi, ebp, unused_esp, ebx, edx, ecx, eax;
    uint32_t int_index, error_code;
    uint32_t eip, cs, eflags, useresp, ss;
} int_registers;

void PIC_EOI(uint8_t interrupt);
void set_int_handler(void handler(int_registers *), uint8_t index);
void init_interrupts(void);

#endif /* INTERRUPTS_H */
