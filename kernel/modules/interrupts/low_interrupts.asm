[BITS 32]

global isr_start_ext
global irq_start_ext

global isr_end_ext
global irq_end_ext

extern call_table

%macro int_no_error 1
global int_%1

int_%1:
	push dword 0							; error code 0
	push dword %1							; interrupt number 
	jmp int_common_handler

%endmacro

%macro int_error 1
global int_%1

int_%1:	
	push dword %1							; interrupt number 
	jmp int_common_handler
	
%endmacro


section .text

int_common_handler:
	pusha
	push ds									
    push es
    push fs
    push gs

	push esp

    mov ax, 0x10							; loading kernel data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

	mov eax, [call_table]
	mov edx, dword [esp + 52]
	shl edx, 2
	add eax, edx							; calculating pointer to handler entry 

	call [eax]								; loading high level int handler

	add esp, 4

	pop gs
	pop fs
	pop es
	pop ds
	popa

	add esp, 8								; removing error code and int number
	iret

align 4

; isr
int_no_error 0
int_no_error 1
int_no_error 2
int_no_error 3
int_no_error 4
int_no_error 5
int_no_error 6
int_no_error 7
int_error 	 8
int_no_error 9
int_error 	 10
int_error 	 11
int_error 	 12
int_error 	 13
int_error 	 14
int_no_error 15
int_no_error 16
int_error 	 17
int_no_error 18
int_no_error 19
int_no_error 20
int_no_error 21
int_no_error 22
int_no_error 23
int_no_error 24
int_no_error 25
int_no_error 26
int_no_error 27
int_no_error 28
int_no_error 29
int_error 	 30
int_no_error 31

; irq
int_no_error 32
int_no_error 33
int_no_error 34
int_no_error 35
int_no_error 36
int_no_error 37
int_no_error 38
int_no_error 39
int_no_error 40
int_no_error 41
int_no_error 42
int_no_error 43
int_no_error 44
int_no_error 45
int_no_error 46
int_no_error 47
