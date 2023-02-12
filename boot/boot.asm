[BITS 16]
[ORG 0x7C00]
; 0x0000:0x7C00 - 0x0000:0x7E00
	jmp start
	nop

;-----------------------------------
; FAT12 descriptor table

%INCLUDE "BIOSparamblock.asm"

;-----------------------------------

start:
	cli
    ; Initial stack setup
	mov ax, 0x7BFF
	mov sp, ax
	sti

	cld

	xor ax, ax
	mov ss, ax
	mov ds, ax
	mov es, ax

	mov [BootDevice], dl

	mov si, BootLoadMsg
	call print_string

    ; Loading second stage of the bootloader

	mov bx, 0x7E00
	mov ax, 0x1

	call logical_to_hts

	mov al, byte [SectorsToLoad]

	call read_from_disk

	jmp 0x7E00

; DS:SI should store string reference
print_string:
	push ax
	mov ah, 0x0E
.loop:
	lodsb
	or al, al
	jz .exit
	int 0x10
	jmp .loop
.exit:
	pop ax
	ret

; AX should store a number to print
print_register:
	push si
	push di
	mov di, .wordToPrint
	call convert_word_to_string

	mov si, .wordToPrint
	call print_string

	pop di
	pop si
	ret

.wordToPrint db 0, 0, 0, 0, 32, 32, 0

; AL should store an input byte
; The result will be placed into ES:DI, ES:(DI + 1)
convert_byte_to_string:
	push ax
	mov ah, al

	and al, 0b11110000
	shr al, 0d4
	add al, 0d48
	cmp al, 0d58
	jb .second
	add al, 0d07

.second:
	stosb
	mov al, ah
	and al, 0b00001111
	add al, 0d48
	cmp al, 0d58
	jb .end
	add al, 0d07

.end:
	stosb
	pop ax
	ret

; AX should store an input word
; The result will be in 4 bytes starting from [ES:DI]
convert_word_to_string:
	push ax

	mov al, ah
	call convert_byte_to_string
	pop ax
	call convert_byte_to_string

	ret

; SI should point to the region start
; DI should have an amount of bytes to print
print_dump:
	push ax
	push di
	push si

	mov si, .nextRow
	call print_string

	pop si
	push si

.loop:
	mov al, byte [si]

	push di
	mov di, .byte
	call convert_byte_to_string
	pop di

	push si
	mov si, .byte
	call print_string
	pop si

	add si, 1
	sub di, 1
	or di, di
	jz short .end
	jmp short .loop
.end:
	pop di
	pop si
	pop ax
	ret

.nextRow db 13, 10, 0
.byte db 0, 0, 32, 0


; AL should store an amount of secotrs to read
; DL should store disk number, DH should store head number, CH - track number
; CL - secotr number (first 6 bits), + 9 b 10 bits are track number
; [ES:BX] points to memory to load

; AH will store return code
; AL will store amount of loaded sectors
read_from_disk:
	mov ah, 0x02
	push di
	mov di, 4

.loop:
	push dx
	push ax

	stc
	int 0x13
	jnc .done

	cmp di, 0
	je .error
	dec di

	pop ax
	pop dx

	jmp .loop

.error:
	call disk_error

.done:
	pop di
	pop dx
	pop di

	clc
	ret

; AH should store disk error
disk_error:
	mov di, DiskErrorCode
    mov al, ah
    call convert_byte_to_string

    mov si, DiskErrorMsg
    call print_string

    mov si, DiskErrorCode
    call print_string

reboot:
	mov si, RebootMsg
	call print_string

	mov ax, 0
	int 16h
	mov ax, 0
	int 19h

; AX should store logical sector
logical_to_hts:
	push ax

	mov dx, 0
	div word [SectorsPerTrack]

    ; Sectors count starts from one
	inc dl
	mov cl, dl

    xor dx, dx
	div word [Sides]
    ; DL stores the shift of the head on this cylinder
	mov dh, dl
    ; AL stores an index of a cylinder
	mov ch, al

	pop ax

	ret

;---------------------------------
; Data

BootDevice						db 0
SectorsToLoad					db 2

;---------------------------------
; Strings

BootLoadMsg						db 'First stage boot...', 13, 10, 0
DiskErrorMsg					db 'Disk read error : ', 0
DiskErrorCode					db 0, 0, 13, 10, 0
RebootMsg						db 'Press any key for reboot...', 0
;---------------------------------

times 510 - ($ - $$) db 0x0
db 0x55, 0xAA
