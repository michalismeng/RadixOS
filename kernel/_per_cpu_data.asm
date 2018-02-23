bits 32
section .text

global _set_cpu_gs
global _per_cpu_read
global _per_cpu_write 

; sets the gs for the executing cpu to a GDT segment
; expects GDT entry offset in stack
_set_cpu_gs:
    mov eax, [esp + 4]      
    mov gs, ax
    ret

; reads a cpu local variable at a given offset
; expects variable byte offset in stack
_per_cpu_read:
	mov ebx, [esp + 4]
	mov eax, [gs:ebx]
	ret

; writes a cpu local variable at a given offset
; expects variable byte offset and the value to write in stack
_per_cpu_write:
    mov ebx, [esp + 4]
    mov eax, [esp + 8]
    mov [gs:ebx], eax
    ret