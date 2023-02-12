.PHONY: all clean

PROJDIR = modules
C_SRC_FILES := $(shell find $(PROJDIR) -type f -name "*.c")
ASM_SRC_FILES := $(shell find $(PROJDIR) -type f -name "*.asm")
OBJFILES := *.o

WARNINGS := -Wall -Wextra -pedantic -Wshadow -Wpointer-arith -Wcast-align \
            -Wwrite-strings -Wmissing-prototypes -Wmissing-declarations \
            -Wredundant-decls -Wnested-externs -Winline -Wno-long-long \
            -Wconversion -Wstrict-prototypes \
			-Wno-unused-function -Wno-unused-parameter

all:
	nasm -f elf modules/kstartup.asm -o kstartup.o
	nasm -f elf modules/interrupts/low_interrupts.asm -o low_interrupts.o
	nasm -f elf modules/multitasking/low_tasks.asm -o low_tasks.o

	i386-elf-gcc -c -std=c99 -O2 -I include -fcommon -ffreestanding $(C_SRC_FILES) $(WARNINGS)
	i386-elf-ld -T link.ld -o kernel.bin $(OBJFILES)

clean:
	rm *.o kernel.bin
