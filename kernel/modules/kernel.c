#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include <config.h>
#include <string.h>
#include <interrupts.h>
#include <devices/terminal.h>
#include <memory.h>
#include <boot.h>
#include <multitasking.h>
#include <kprintf.h>

#ifdef DEBUG
#include <debug.h>
#endif

void kmain(boot_info *boot);

void kmain(boot_info *boot)
{
#ifdef DEBUG
	int32_t debug_status = init_debug();

	dbg_kprintf("\
COM logging started\n\
Boot info:\n\
Kernel size: %#x\n\
Low memory: %#x, reserved: %#x\n\
High memory %#x, reserved: %#x\n",
	boot->kernel_size,\
	boot->map.low_mem, boot->map.low_mem_reserved,\
	boot->map.high_mem, boot->map.high_mem_reserved);
#endif

	init_mm(&boot->map);

	init_terminal();

#ifdef DEBUG
	if(debug_status == -1)
    {
		kprintf("Debug port activation error, data can't be send to external log file...\n");
    }
    else
    {
		kprintf("COM port activated...\n");
    }
#endif

	kprintf("Memory manager started...\n");
	kprintf("Terminal started...\n");

	init_interrupts();
	kprintf("Interrupt manager started...\n");

	init_multitasking();
	kprintf("Multitasking manager started...\n");

	while(true)
	{
		sleep(1);
	}
}
