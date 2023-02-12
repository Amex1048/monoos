[BITS 32]

global switch_task
global init_tss

extern tss
extern current_task;

struc tss_t
    link: resw 1
    reserve: resw 1
    esp0: resd 1
    ss0: resw 1
endstruc

init_tss:
    push eax

    mov word [tss + ss0], 0x10

    mov eax, 0x28
    ltr ax
    pop eax
    ret

switch_task:
    push ebx
    push esi
    push edi
    push ebp

    mov esi, [current_task]

    ; Store previous task stack pointer
    mov [esi], esp

    mov esi, [esp + 4 * 5]

    ; Load next task struct
    mov [current_task], esi

    ; Load next task stack
    mov esp, [esi]
    mov edi, [esi + 4]

    ; Load next task cr3
    mov cr3, edi

    pop ebp
    pop edi
    pop esi
    pop ebx

    ret
