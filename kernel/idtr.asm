bits 32 

section .text

global _flushIDT

_flushIDT:
	mov eax, [esp + 4]		; get idtr pointer
	lidt [eax]
	
    ret

; define the ISR templates

%macro  ISR_NOERRORCODE 1
global isr%1
isr%1:
    cli

	push 0				; push a dummy error code value
	push %1				; push the interrupt number

	jmp isr_common_stub

%endmacro

%macro  ISR_ERRORCODE 1
global isr%1
isr%1:
    cli
	
	push %1				; push the interrupt number

	jmp isr_common_stub

%endmacro

%macro  IRQ 2
global irq%1
irq%1:
    cli

	push 0				; push a dummy error code value
	push %2				; push the interrupt number

	jmp irq_common_stub

%endmacro

%macro  APIC_IRQ 2
global apic_irq%1
apic_irq%1:
    cli

	push 0				; push a dummy error code value
	push %2				; push the interrupt number

	jmp apic_irq_common_stub

%endmacro

; using the above macros spawn the required ISRs

ISR_ERRORCODE 8
ISR_ERRORCODE 10
ISR_ERRORCODE 11
ISR_ERRORCODE 12
ISR_ERRORCODE 13
ISR_ERRORCODE 14

ISR_NOERRORCODE 0
ISR_NOERRORCODE 1
ISR_NOERRORCODE 2
ISR_NOERRORCODE 3
ISR_NOERRORCODE 4
ISR_NOERRORCODE 5
ISR_NOERRORCODE 6
ISR_NOERRORCODE 7
ISR_NOERRORCODE 9
ISR_NOERRORCODE 15
ISR_NOERRORCODE 16
ISR_NOERRORCODE 17
ISR_NOERRORCODE 18
ISR_NOERRORCODE 19
ISR_NOERRORCODE 20
ISR_NOERRORCODE 21
ISR_NOERRORCODE 22
ISR_NOERRORCODE 23
ISR_NOERRORCODE 24
ISR_NOERRORCODE 25
ISR_NOERRORCODE 26
ISR_NOERRORCODE 27
ISR_NOERRORCODE 28
ISR_NOERRORCODE 29
ISR_NOERRORCODE 30
ISR_NOERRORCODE 31

ISR_NOERRORCODE 128

IRQ 0,  224
IRQ 1,  225
IRQ 2,  226
IRQ 3,  227
IRQ 4,  228
IRQ 5,  229
IRQ 6,  230
IRQ 7,  231
IRQ 8,  232
IRQ 9,  233
IRQ 10, 234
IRQ 11, 235
IRQ 12, 236
IRQ 13, 237
IRQ 14, 238
IRQ 15, 239

APIC_IRQ 0,  64
APIC_IRQ 1,  65
APIC_IRQ 2,  66
APIC_IRQ 3,  67
APIC_IRQ 4,  68
APIC_IRQ 5,  69
APIC_IRQ 6,  70
APIC_IRQ 7,  71
APIC_IRQ 8,  72
APIC_IRQ 9,  73
APIC_IRQ 10, 74
APIC_IRQ 11, 75
APIC_IRQ 12, 76
APIC_IRQ 13, 77
APIC_IRQ 14, 78
APIC_IRQ 15, 79

extern isr_handler
extern irq_handler
extern acpi_irq_handler

; ISR common stub calls the C function isr_handler to route a cpu exception
isr_common_stub:
				; already pushed error_code, interrupt no
	pushad		; push eax, ecx, edx, ebx, esp, ebp, esi, edi

	mov ax, ds
	push eax		; push "ds"

	mov ax, 10h
	mov ds, ax
	mov es, ax
	mov fs, ax
	;mov gs, ax			TODO: Consider saving gs and restoring it after the interrupt

	call isr_handler
	
	pop eax

	mov ds, ax
	mov es, ax
	mov fs, ax
	;mov gs, ax

	popad
	add esp, 8

	iret        ; pop CS, EIP, EFLAGS, SS, and ESP


; IRQ common stub calls the C function irq_handler to route a hardware interrupt
irq_common_stub:
	pushad

	mov ax, ds
	push eax

	mov ax, 10h
	mov ds, ax
	mov es, ax
	mov fs, ax
	;mov gs, ax

	call irq_handler

	pop eax
	mov ds, ax
	mov es, ax
	mov fs, ax
	;mov gs, ax

	popad
	add esp, 8

	iretd

; APIC IRQ common stub calls the C function irq_handler to route a hardware interrupt
apic_irq_common_stub:
	pushad

	mov ax, ds
	push eax

	mov ax, 10h
	mov ds, ax
	mov es, ax
	mov fs, ax
	;mov gs, ax

	call acpi_irq_handler

	pop eax
	mov ds, ax
	mov es, ax
	mov fs, ax
	;mov gs, ax

	popad
	add esp, 8

	iretd

