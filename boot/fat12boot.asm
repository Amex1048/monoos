%INCLUDE "boot.asm"

second_boot_start:
	mov si, LoadMsg
	call print_string

    ; Check for i386
	mov ax, 0x7202
	push ax
	popf
	pushf
	pop bx
	cmp ax, bx

	je .i386_confirmed

	mov si, i386Error
	call print_string

	cli
	hlt

.i386_confirmed:
	call get_mem_count

	call open_A20

    ; Calculating root directory position
	mov ax, word [ReservedForBoot]
	mov cx, word [BytesPerSector]
	mul cx

	add ax, 0x7C00
	mov word [DiskRAMPos], ax

    ; FAT table and root directory should be less than 65 sectors 512 bytes each (33280 bytes)
	mov bx, ax

	xor ax, ax

	mov al, byte [NumberOfFats]

	mov cx, word [SectorsPerFat]
	mul cx

	add ax, word [ReservedForBoot]
	mov word [RootDirPointer], ax


	call logical_to_hts
	mov dl, byte [BootDevice]

    ; Reading root directory
	call read_from_disk

	mov si, KernelName
	call find_file

	add di, 28
    ; Calculating kernel size
	mov dword eax, [di]
	mov dword [KernelSize], eax
	sub di, 28

    ; Calculating file cluster pointer
	add di, 26
	mov ax, [di]
	mov word [KernelFATSector], ax

    ; Loading FAT table
	mov ax, word [RootDirEntries]
	mov cx, 32
	mul cx
	add ax, word [DiskRAMPos]
	mov word [FATRAMPos], ax
	mov bx, ax

	mov ax, word [ReservedForBoot]

	call logical_to_hts
	mov dl, byte [BootDevice]

	mov al, byte [SectorsPerFat]

	call read_from_disk

	mov si, word [KernelFATSector]

	mov bx, 0xFFFF
	mov es, bx
	mov bx, 0x0010

	call load_file

	xor bx, bx
	mov es, bx
	mov ds, bx
	mov dx, bx

	mov si, KernelLD
	call print_string

	lgdt [gdtp]

	cli

	mov eax, cr0
	or eax, 1
	mov cr0, eax

    ; Flushing instruction pipeline
	jmp 0x8:start_protected

[BITS 32]

start_protected:
	mov eax, 16
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax

	mov dword ebx, BootInfo
	push ebx

	jmp 0x100000

[BITS 16]
;------------------------------------------------

get_mem_count:
	clc
	int 0x12
	jc .error

	mov word [LowMem], ax

	mov si, .LowMemMsg
	call print_string
	call print_register

	mov si, .NextLine
	call print_string

	clc
	mov ah, 0x88
	int 0x15
	jc .error

	mov word [HighMem], ax

	mov si, .HighMemMsg
	call print_string
	call print_register
	mov si, .NextLine
	call print_string

	ret

.error:
	mov si, MemFuncError
	call print_string
	call reboot

.LowMemMsg db 'Low memory : ', 0
.HighMemMsg db 'High memory : ', 0
.NextLine db 13, 10, 0

; SI should have pointer to name string
; DI will store pointer to the file directory
find_file:
	push dx
	push cx

	mov dx, word [RootDirEntries]
	mov di, word [DiskRAMPos]

.loop:
	push si
	push di

	mov cx, 11
	rep cmpsb
	je short .found

	pop di
	pop si

	add di, 32
	dec dx
	or dx, dx
	jz short .not_found
	jmp short .loop

.found:
	pop di
	pop si

	pop cx
	pop dx
	ret

.not_found:
	mov si, FileNotFound
	call print_string
	xor ax, ax
	int 16h
	xor ax, ax
	int 19h

; SI should store file cluster start
; ES:BX should store pointer to loading buffer
load_file:
	pusha
	mov word [.CurrentCluster], si

	mov ax, word [BytesPerSector]
	xor dx, dx
	mov dl, byte [SectorsPerCluster]
	mul dx
	mov word [.ClusterSize], ax

	xor dx, dx
	mov ax, word [RootDirEntries]
	mov cx, 16
	div cx

	add ax, word [RootDirPointer]
	mov word [.DataPointer], ax

.load_sector:
	mov ax, word [.CurrentCluster]
	add ax, word [.DataPointer]
	sub ax, 2

	call logical_to_hts

	mov dl, byte [BootDevice]
	xor ax, ax
	mov al, byte [SectorsPerCluster]

	call read_from_disk

	add bx, word [.ClusterSize]

	mov ax, word [.CurrentCluster]
	mov cx, 3
	mul cx
	mov cx, 2
	div cx

	add ax, word [FATRAMPos]

	mov di, ax
	mov ax, [di]

	or dx, dx
	jz short .even

	shr ax, 4
	jmp short .counted

.even:
	and ax, 0x0FFF

.counted:
	mov word [.CurrentCluster], ax
	cmp ax, 0x0FF8
	jae short .end

	jmp short .load_sector

.end:
	popa
	ret

;.FileLoadMsg			db 'File loaded!', 0
.ClusterSize			    dw 0
.CurrentCluster				dw 0
.DataPointer			    dw 0					; Sectors

%INCLUDE "OpenA20.asm"

;---------------------------------
; Data

DiskRAMPos				dw 0					; Bytes
RootDirPointer			dw 0					; Sectors
FATRAMPos				dw 0					; Bytes
KernelName				db 'KERNEL  BIN'
DiskDriverName			db 'DISKDRV BIN'
KernelFATSector			dw 0					; Bytes
DiskFATSector			dw 0					; Bytes

BootInfo:
KernelSize				dd 0					; Bytes
MemMap:
LowMem					dw 0					; KBytes
LowMemReserved			dw 0					; KBytes
HighMem					dw 0					; KBytes
HihMemReserved			dw 0					; KBytes

;---------------------------------
; Strings

i386Error    db 'Required i386 or better', 13, 10, 0
LoadMsg      db 'Second stage boot...', 13, 10, 0
KernelLD     db 'Kernel loaded...', 13, 10, 0
MemFuncError db 'Memory function error...', 13, 10, 0
FileNotFound db 'File not found...', 13, 10, 0

;---------------------------------
; Descriptor table
align 16

gdt:
	dq 0										; NULL
	dq 0x00CF9A000000FFFF						; Code
	dq 0x00CF92000000FFFF						; Data

gdtp:
	dw $ - gdt - 1
	dd gdt

;---------------------------------

times 1536 - ($ - $$) db 0x0
