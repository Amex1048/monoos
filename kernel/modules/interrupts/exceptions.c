#include <stddef.h>
#include <stdint.h>

#include <interrupts.h>
#include <debug.h>
#include <kprintf.h>
#include <hal.h>

#include "exceptions.h"

static void reserved_interrupt(int_registers *regs)
{
    kprintf("Reserved interrupt called or happened on adress %p\n", regs->eip);
    kprintf("Code segment: %hx\n Stack pointer: %p\n Stack segment: %hx\n",\
            regs->cs, regs->useresp, regs->ss);

#ifdef DEBUG
    dbg_bochs();
#endif
}

static void divide_by_zero(int_registers *regs)
{
    kprintf("Divide by zero exception on adress %p\n", regs->eip);

#ifdef DEBUG
    dbg_bochs();
#endif
}

static void invalid_opcode(int_registers *regs)
{
    kprintf("Invalid opcode on adress %p\n", regs->eip);

#ifdef DEBUG
    dbg_bochs();
#endif
}

static void double_fault(int_registers *regs)
{
    kprintf("Double fault exception on adress(may be invalid) %p\n", regs->eip);
    kprintf("Code segment: %hx\n Stack pointer: %p\n Stack segment: %hx\n Halting...",\
            regs->cs, regs->useresp, regs->ss);

    hlt();
}

static void general_protection_fault(int_registers *regs)
{
    kprintf("General protection error on adress %p\n", regs->eip);
    kprintf("Code segment: %hx\n Stack pointer: %p\n Stack segment: %hx\n",\
            regs->cs, regs->useresp, regs->ss);
    kprintf("Error code: %x\n", regs->error_code);

#ifdef DEBUG
    dbg_bochs();
#endif
}

static void page_fault(int_registers *regs)
{
    kprintf("Page fault error on adress %p\n", regs->eip);
    kprintf("Code segment: %hx\n Stack pointer: %p\n Stack segment: %hx\n",\
            regs->cs, regs->useresp, regs->ss);
    kprintf("Error code: %x\n", regs->error_code);

#ifdef DEBUG
    dbg_bochs();
#endif
}

void init_exceptions(void)
{
    for(uint16_t i = 0; i < 256; i++)
    {
        set_int_handler(reserved_interrupt, (uint8_t)i);
    }

    set_int_handler(divide_by_zero, 0x0);
    set_int_handler(invalid_opcode, 0x6);
    set_int_handler(double_fault, 0x8);
    set_int_handler(general_protection_fault, 0xD);
    set_int_handler(page_fault, 0xE);
}