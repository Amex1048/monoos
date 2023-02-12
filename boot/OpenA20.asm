open_A20:
	call check_A20
	cmp al, 1
	je .A20_opened

    ; Trying different A20 activation methods
	mov ax,	2401h
	int 15h

	call check_A20
	cmp al, 1
	je .A20_opened

	call keyboard_A20

	jmp .A20_opened

	ret

.A20_opened:
	mov si, A20Opened
	call print_string
	ret

check_A20:
    pushf
    push ds
    push es
    push di
    push si

    cli

    xor ax, ax
    mov es, ax

    not ax
    mov ds, ax

    mov di, 0x0500
    mov si, 0x0510

    mov al, byte [es:di]
    push ax

    mov al, byte [ds:si]
    push ax

    mov byte [es:di], 0x00
    mov byte [ds:si], 0xFF

    cmp byte [es:di], 0xFF

    pop ax
    mov byte [ds:si], al

    pop ax
    mov byte [es:di], al

    mov ax, 0
    je check_a20_exit

    mov ax, 1

check_a20_exit:
    pop si
    pop di
    pop es
    pop ds
    popf

    ret

[BITS 32]
keyboard_A20:
    cli

    call    .a20wait
    mov     al,0xAD
    out     0x64,al

    call    .a20wait
    mov     al,0xD0
    out     0x64,al

    call    .a20wait2
    in      al,0x60
    push    eax

    call    .a20wait
    mov     al,0xD1
    out     0x64,al

    call    .a20wait
    pop     eax
    or      al,2
    out     0x60,al

    call    .a20wait
    mov     al,0xAE
    out     0x64,al

    call    .a20wait
    sti
    ret

.a20wait:
    in      al,0x64
    test    al,2
    jnz     .a20wait
    ret

.a20wait2:
    in      al,0x64
    test    al,1
    jz      .a20wait2
    ret

[BITS 16]
;---------------------------------
; Data

;---------------------------------
; Strings

A20OpenError db 'A20 open error...', 13, 10, 0
A20Opened db 'A20 opened...', 13, 10, 0

;---------------------------------
