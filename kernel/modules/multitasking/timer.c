#include <stddef.h>
#include <stdint.h>

#include <config.h>
#include <hal.h>
#include <interrupts.h>
#include <multitasking.h>
#include <debug.h>
#include <kprintf.h>

#include "mtdef.h"

#define PIC_OFFSET 0
#define PIT_FREQ 1193180

#define CHANNEL_0 0x40
#define CHANNEL_1 0x41
#define CHANNEL_2 0x42
#define CHANNEL_MODE 0x43

static void interrupt_handler(int_registers *regs)
{
	nanoseconds_since_boot += (1000000 / TICK_HZ);

#ifdef DEBUG_MT
	//dbg_kprintf("\nnanoseconds_since_boot %d\n", nanoseconds_since_boot);
#endif

	task_t *awaked_task = get_awaked_task();
	while (awaked_task != NULL)
	{
		awaked_task->state = READY;
		push_front(awaked_task, &ready_tasks);
		awaked_task = get_awaked_task();
	}

	PIC_EOI(0);

	schedule();
#ifdef DEBUG_MT
	//dbg_kprintf("\nPIT interrupt end\n");
#endif
}

static void RTC_handler(int_registers *regs)
{
	outb(MPIC_COMMAND, 0x0b);

	uint8_t isr = inb(MPIC_COMMAND);

	if(isr != 0)
	{
		kprintf("Real RTC\n");
		PIC_EOI(7);
	}
	else 
	{
#ifdef DEBUG
		dbg_kprintf("\nSpurious INT\n");
#endif
		kprintf("Spurious INT\n");
	}
}

void init_timer(void)
{
	set_int_handler(interrupt_handler, 32);
	set_int_handler(RTC_handler, 39);

	nanoseconds_since_boot = 0;

	uint32_t divisor = PIT_FREQ / TICK_HZ;

	outb(CHANNEL_MODE, 0x36);
	outb(CHANNEL_0, (uint8_t)(divisor & 0xFF));
	outb(CHANNEL_0, (uint8_t)(divisor >> 8));
}