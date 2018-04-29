bits 32
section .text

global _set_cpu_gs
global _per_cpu_read
global _per_cpu_write 

global get_stack
get_stack:
    mov eax, esp
    ret

; sets the gs for the executing cpu to a GDT segment
; expects GDT entry offset in stack
_set_cpu_gs:
    mov eax, [esp + 4]      
    mov gs, ax
    ret

; reads a cpu local variable at a given offset
; expects variable byte offset in stack
_per_cpu_read:
	mov eax, [esp + 4]
	mov eax, [gs:eax]
	ret

; writes a cpu local variable at a given offset
; expects variable byte offset and the value to write in stack
_per_cpu_write:
    push ebx                    ; save ebx as we will trash it

    mov ebx, [esp + 4 + 4]      ; since we pushed ebx, we need to show 4 bytes upper in the stack
    mov eax, [esp + 8 + 4]
    mov [gs:ebx], eax

    pop ebx                     ; restore ebx
    ret