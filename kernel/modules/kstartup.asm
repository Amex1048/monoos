[BITS 32]

extern kmain
extern _total_size

global tss_entry
global tss

section .text

; EBX should store memory map pointer
kstart:
	call prepare_paging

	mov eax, 0x1000
    ; Load page dir pointer
	mov cr3, eax
	mov eax, cr0
	or eax, 0x80000000
	mov cr0, eax

	lgdt [gdtp]

	pop ebx

	mov eax, 0xFFBFEFFF
	mov esp, eax

	mov dword [ebx], _total_size
	mov word [ebx + 6], 24
	mov word [ebx + 10], 64

	push ebx

	call kmain

	cli
	hlt

;	----------
;	Page Dir	4 kb
;   ----------
;	Page tables 4mb - 4kb
;	---------
;	Stack		4kb
;   ----------
;	*HOLE*
;	----------
;	Heap		512mb - 4 mb - 12kb - 64kb
;	----------
;	Kernel		16 mb


prepare_paging:
	xor eax, eax
	mov edi, 0x1000
	mov ecx, 4 * 4096 / 4
	rep stosd

	mov dword [0x1FFC], 0x1000 + 0b11
	mov dword [0x1E00], 0x2000 + 0b11
	mov dword [0x1FF8], 0x3000 + 0b11
	mov dword [0x1000], 0x5000 + 0b11

	mov dword eax, _total_size
	mov ecx, 4096
	div ecx
	cmp edx, 0
	je .no_rem
	inc eax
.no_rem:
	mov ecx, eax

	mov edi, 0x2000
	mov eax, 0x100000 + 0b11
    ; Mount kernel bin (64KB) to 0xE0000000
	call .fill

	mov dword [0x3FF8], 0x4000 + 0b11

	mov edi, 0x5000
	mov ecx, 272
	mov eax, 0x0 + 0b11
    ; Mount first 1MB + 64KB
	call .fill

	ret

.fill:
	stosd
	add eax, 0x1000
	dec ecx
	jz .filled
	jmp .fill

.filled:
	ret



section .data

align 8

gdt:
	dq 0										; Null
	dq 0x00CF9A000000FFFF						; Kernel Code
	dq 0x00CF92000000FFFF						; Kernel Data
	dq 0x00CFFA000000FFFF						; User Code
	dq 0x00CFF2000000FFFF						; User Data
tss_entry:
	resb 8										; TSS

gdtp:
	dw $ - gdt - 1
	dd gdt

align 4

tss:
	resb 96

