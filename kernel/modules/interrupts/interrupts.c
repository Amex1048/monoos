#include <stddef.h>
#include <stdint.h>

#include <hal.h>
#include <string.h>
#include <interrupts.h>
#include <memory.h>

#include "exceptions.h"

// Definitions

#define INT_KERNEL_TYPE 0x8E

// ISR
extern size_t int_0;
extern size_t int_1;
extern size_t int_2;
extern size_t int_3;
extern size_t int_4;
extern size_t int_5;
extern size_t int_6;
extern size_t int_7;
extern size_t int_8;
extern size_t int_9;
extern size_t int_10;
extern size_t int_11;
extern size_t int_12;
extern size_t int_13;
extern size_t int_14;
extern size_t int_15;
extern size_t int_16;
extern size_t int_17;
extern size_t int_18;
extern size_t int_19;
extern size_t int_20;
extern size_t int_21;
extern size_t int_22;
extern size_t int_23;
extern size_t int_24;
extern size_t int_25;
extern size_t int_26;
extern size_t int_27;
extern size_t int_28;
extern size_t int_29;
extern size_t int_30;
extern size_t int_31;

// ISR
extern size_t int_32;
extern size_t int_33;
extern size_t int_34;
extern size_t int_35;
extern size_t int_36;
extern size_t int_37;
extern size_t int_38;
extern size_t int_39;
extern size_t int_40;
extern size_t int_41;
extern size_t int_42;
extern size_t int_43;
extern size_t int_44;
extern size_t int_45;
extern size_t int_46;
extern size_t int_47;

// Interrupt description table pointer
typedef struct idt_p {
	uint16_t limit;
	void *base;
} __attribute__((packed)) idt_p;

typedef struct int_desc
{
	uint16_t address0_15;
	uint16_t selector;
	uint8_t reserved;
	uint8_t type;
	uint16_t address16_31;
} __attribute__((packed)) int_desc;

// Variables

int_desc *idt_table;
size_t *call_table;

// Private

static void io_wait(void)
{
	for (volatile uint16_t i = 0; i < 10000; ++i)
	{
	}
}

static void set_int_lowlevelhandler(int_desc *table, void *handler, uint8_t index, uint8_t type)
{
	uint32_t flags;
	__asm__ volatile("pushf; \
					  popl %0; \
					  cli"
					 : "=a"(flags));

	table[index].selector = 8;
	table[index].reserved = 0;
	table[index].type = type;
	table[index].address16_31 = (uint16_t)((size_t)handler >> 16);
	table[index].address0_15 = (size_t)handler & 0xFFFFU;

	__asm__ volatile("pushl %0; \
					  popf"
					 :
					 : "a"(flags));
}

// Public

void PIC_EOI(uint8_t interrupt)
{
	if (interrupt > 7)
	{
		outb(SPIC_COMMAND, EOI);
	}

	outb(MPIC_COMMAND, EOI);
}

void set_int_handler(void handler(int_registers *), uint8_t index)
{
	uint32_t flags;
	__asm__ volatile("pushf; \
					  popl %0; \
					  cli"
					 : "=a"(flags));

	call_table[index] = (size_t)handler;

	__asm__ volatile("pushl %0; \
					  popf" ::"a"(flags));
}

void init_interrupts(void)
{
	idt_table = (int_desc *)kmalloc(sizeof(int_desc) * 256, MM_STANDART);
	memset(idt_table, 0, 256 * sizeof(int_desc)); // Очищаем область для таблицы IDT

	call_table = (size_t *)kmalloc(sizeof(size_t) * 256, MM_STANDART);
	memset(call_table, 0, 256 * sizeof(size_t));

	set_int_lowlevelhandler(idt_table, &int_0, 0, INT_KERNEL_TYPE);
	set_int_lowlevelhandler(idt_table, &int_1, 1, INT_KERNEL_TYPE);
	set_int_lowlevelhandler(idt_table, &int_2, 2, INT_KERNEL_TYPE);
	set_int_lowlevelhandler(idt_table, &int_3, 3, INT_KERNEL_TYPE);
	set_int_lowlevelhandler(idt_table, &int_4, 4, INT_KERNEL_TYPE);
	set_int_lowlevelhandler(idt_table, &int_5, 5, INT_KERNEL_TYPE);
	set_int_lowlevelhandler(idt_table, &int_6, 6, INT_KERNEL_TYPE);
	set_int_lowlevelhandler(idt_table, &int_7, 7, INT_KERNEL_TYPE);
	set_int_lowlevelhandler(idt_table, &int_8, 8, INT_KERNEL_TYPE);
	set_int_lowlevelhandler(idt_table, &int_9, 9, INT_KERNEL_TYPE);
	set_int_lowlevelhandler(idt_table, &int_10, 10, INT_KERNEL_TYPE);
	set_int_lowlevelhandler(idt_table, &int_11, 11, INT_KERNEL_TYPE);
	set_int_lowlevelhandler(idt_table, &int_12, 12, INT_KERNEL_TYPE);
	set_int_lowlevelhandler(idt_table, &int_13, 13, INT_KERNEL_TYPE);
	set_int_lowlevelhandler(idt_table, &int_14, 14, INT_KERNEL_TYPE);
	set_int_lowlevelhandler(idt_table, &int_15, 15, INT_KERNEL_TYPE);
	set_int_lowlevelhandler(idt_table, &int_16, 16, INT_KERNEL_TYPE);
	set_int_lowlevelhandler(idt_table, &int_17, 17, INT_KERNEL_TYPE);
	set_int_lowlevelhandler(idt_table, &int_18, 18, INT_KERNEL_TYPE);
	set_int_lowlevelhandler(idt_table, &int_19, 19, INT_KERNEL_TYPE);
	set_int_lowlevelhandler(idt_table, &int_20, 20, INT_KERNEL_TYPE);
	set_int_lowlevelhandler(idt_table, &int_21, 21, INT_KERNEL_TYPE);
	set_int_lowlevelhandler(idt_table, &int_22, 22, INT_KERNEL_TYPE);
	set_int_lowlevelhandler(idt_table, &int_23, 23, INT_KERNEL_TYPE);
	set_int_lowlevelhandler(idt_table, &int_24, 24, INT_KERNEL_TYPE);
	set_int_lowlevelhandler(idt_table, &int_25, 25, INT_KERNEL_TYPE);
	set_int_lowlevelhandler(idt_table, &int_26, 26, INT_KERNEL_TYPE);
	set_int_lowlevelhandler(idt_table, &int_27, 27, INT_KERNEL_TYPE);
	set_int_lowlevelhandler(idt_table, &int_28, 28, INT_KERNEL_TYPE);
	set_int_lowlevelhandler(idt_table, &int_29, 29, INT_KERNEL_TYPE);
	set_int_lowlevelhandler(idt_table, &int_30, 30, INT_KERNEL_TYPE);
	set_int_lowlevelhandler(idt_table, &int_31, 31, INT_KERNEL_TYPE);

	set_int_lowlevelhandler(idt_table, &int_32, 32, INT_KERNEL_TYPE);
	set_int_lowlevelhandler(idt_table, &int_33, 33, INT_KERNEL_TYPE);
	set_int_lowlevelhandler(idt_table, &int_34, 34, INT_KERNEL_TYPE);
	set_int_lowlevelhandler(idt_table, &int_35, 35, INT_KERNEL_TYPE);
	set_int_lowlevelhandler(idt_table, &int_36, 36, INT_KERNEL_TYPE);
	set_int_lowlevelhandler(idt_table, &int_37, 37, INT_KERNEL_TYPE);
	set_int_lowlevelhandler(idt_table, &int_38, 38, INT_KERNEL_TYPE);
	set_int_lowlevelhandler(idt_table, &int_39, 39, INT_KERNEL_TYPE);
	set_int_lowlevelhandler(idt_table, &int_40, 40, INT_KERNEL_TYPE);
	set_int_lowlevelhandler(idt_table, &int_41, 41, INT_KERNEL_TYPE);
	set_int_lowlevelhandler(idt_table, &int_42, 42, INT_KERNEL_TYPE);
	set_int_lowlevelhandler(idt_table, &int_43, 43, INT_KERNEL_TYPE);
	set_int_lowlevelhandler(idt_table, &int_44, 44, INT_KERNEL_TYPE);
	set_int_lowlevelhandler(idt_table, &int_45, 45, INT_KERNEL_TYPE);
	set_int_lowlevelhandler(idt_table, &int_46, 46, INT_KERNEL_TYPE);
	set_int_lowlevelhandler(idt_table, &int_47, 47, INT_KERNEL_TYPE);

	init_exceptions();

	volatile idt_p idtp = {256 * sizeof(int_desc), (void *)idt_table};
	__asm__ volatile("lidt (,%0,)" ::"r"(&idtp));

	// PIC init
	outb(MPIC_COMMAND, 0x11);
	io_wait();
	outb(SPIC_COMMAND, 0x11);
	io_wait();

	outb(MPIC_DATA, MPIC_OFFSET);
	io_wait();
	outb(SPIC_DATA, SPIC_OFFSET);
	io_wait();

	outb(MPIC_DATA, 4);
	io_wait();
	outb(SPIC_DATA, 2);
	io_wait();

	outb(MPIC_DATA, 1);
	io_wait();
	outb(SPIC_DATA, 1);
	io_wait();
}
