bits 32 

section .text

global _flushTSS
global _switch_to_user_mode

_flushTSS:
	mov eax, [esp + 4]		; get tss selector
	ltr ax
    ret

extern do_user

_switch_to_user_mode:
    cli                 ; interrupts will be re-enabled using iret
    mov ax, 0x23
    mov ds, ax
    mov es, ax
    mov fs, ax
    ; mov gs, ax

    mov eax, 0x700000
    add eax, 4096
    push 0x23           ; user mode SS
    push eax
    pushfd
    pop eax
    ; or eax, 0x200       ; set the IF bit -- enable this
    push eax
    push 0x1B           ; user mode CS
    push do_user        ; user code EIP

    iret                
    ; we should now execute the do_user function