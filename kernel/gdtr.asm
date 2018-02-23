bits 32 

section .text

global _flushGDT
global _print_A

_flushGDT:
	mov eax, [esp + 4]		; get gdtr pointer
	lgdt [eax]
	
	jmp 0x08:.reload_segments

	.reload_segments:
		
		.reload_cs:
			mov ax, 0x10
			mov ds, ax
			mov es, ax
			mov fs, ax
			mov gs, ax
			mov ss, ax

			ret
_print_A:
	mov ebx, 0xB8000
	mov byte [ebx], 'A'