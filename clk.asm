	.file	"clock.c"
	.intel_syntax noprefix
	.text
	.section	.rodata
.LC0:
	.string	"Sunday"
.LC1:
	.string	"Monday"
.LC2:
	.string	"Tuesday"
.LC3:
	.string	"Wednesday"
.LC4:
	.string	"Thursday"
.LC5:
	.string	"Friday"
.LC6:
	.string	"Saturday"
	.data
	.align 4
	.type	days_of_week, @object
	.size	days_of_week, 28
days_of_week:
	.long	.LC0
	.long	.LC1
	.long	.LC2
	.long	.LC3
	.long	.LC4
	.long	.LC5
	.long	.LC6
	.text
	.type	update_system_time, @function
update_system_time:
.LFB0:
	.cfi_startproc
	push	ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	mov	ebp, esp
	.cfi_def_cfa_register 5
	sub	esp, 24
	call	get_gst
	add	eax, 52
	mov	DWORD PTR [ebp-12], eax
	mov	eax, DWORD PTR [ebp-12]
	movzx	eax, WORD PTR [eax]
	lea	edx, [eax+10]
	mov	eax, DWORD PTR [ebp-12]
	mov	WORD PTR [eax], dx
	mov	eax, DWORD PTR [ebp-12]
	movzx	eax, WORD PTR [eax]
	cmp	ax, 999
	jbe	.L3
	mov	eax, DWORD PTR [ebp-12]
	mov	WORD PTR [eax], 0
	mov	eax, DWORD PTR [ebp-12]
	movzx	eax, BYTE PTR [eax+2]
	lea	edx, [eax+1]
	mov	eax, DWORD PTR [ebp-12]
	mov	BYTE PTR [eax+2], dl
	mov	eax, DWORD PTR [ebp-12]
	movzx	eax, BYTE PTR [eax+2]
	cmp	al, 59
	jbe	.L3
	mov	eax, DWORD PTR [ebp-12]
	mov	BYTE PTR [eax+2], 0
	mov	eax, DWORD PTR [ebp-12]
	movzx	eax, BYTE PTR [eax+3]
	lea	edx, [eax+1]
	mov	eax, DWORD PTR [ebp-12]
	mov	BYTE PTR [eax+3], dl
	mov	eax, DWORD PTR [ebp-12]
	movzx	eax, BYTE PTR [eax+3]
	cmp	al, 60
	jne	.L3
	mov	eax, DWORD PTR [ebp-12]
	mov	BYTE PTR [eax+3], 0
	mov	eax, DWORD PTR [ebp-12]
	movzx	eax, BYTE PTR [eax+4]
	lea	edx, [eax+1]
	mov	eax, DWORD PTR [ebp-12]
	mov	BYTE PTR [eax+4], dl
	mov	eax, DWORD PTR [ebp-12]
	movzx	eax, BYTE PTR [eax+4]
	cmp	al, 24
	jne	.L3
	mov	eax, DWORD PTR [ebp-12]
	mov	BYTE PTR [eax+4], 0
	mov	eax, DWORD PTR [ebp-12]
	movzx	eax, BYTE PTR [eax+5]
	lea	edx, [eax+1]
	mov	eax, DWORD PTR [ebp-12]
	mov	BYTE PTR [eax+5], dl
	mov	eax, DWORD PTR [ebp-12]
	movzx	eax, BYTE PTR [eax+9]
	lea	edx, [eax+1]
	mov	eax, DWORD PTR [ebp-12]
	mov	BYTE PTR [eax+9], dl
.L3:
	nop
	leave
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE0:
	.size	update_system_time, .-update_system_time
	.section	.rodata
.LC7:
	.string	"kernel stack = %h %h"
	.text
	.type	timer_callback, @function
timer_callback:
.LFB1:
	.cfi_startproc
	push	ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	mov	ebp, esp
	.cfi_def_cfa_register 5
	sub	esp, 24
	mov	eax, DWORD PTR x.2301
	add	eax, 1
	mov	DWORD PTR x.2301, eax
	call	get_current_thread
	sub	esp, 8
	push	DWORD PTR [ebp+8]
	push	eax
	call	scheduler_save_thread
	add	esp, 16
	call	get_current_thread
	mov	edx, DWORD PTR [eax+32]
	mov	eax, DWORD PTR [ebp+8]
	mov	eax, DWORD PTR [eax+28]
	sub	esp, 4
	push	edx
	push	eax
	push	OFFSET FLAT:.LC7
	call	printfln
	add	esp, 16
	call	get_gst
	mov	eax, DWORD PTR [eax+8]
	sub	esp, 12
	push	eax
	call	lapic_send_eoi
	add	esp, 16
	call	get_current_thread
	mov	eax, DWORD PTR [eax+32]
	mov	DWORD PTR [ebp-12], eax
	call	get_current_thread
	lea	edx, [eax+4]
	mov	eax, DWORD PTR [ebp-12]
	sub	esp, 4
	push	68
	push	edx
	push	eax
	call	memcpy
	add	esp, 16
	sub	esp, 12
	push	0
	call	per_cpu_read
	add	esp, 16
	mov	edx, eax
	mov	eax, edx
	add	eax, eax
	add	eax, edx
	sal	eax, 3
	lea	edx, [eax+40]
	mov	eax, DWORD PTR [ebp-12]
/APP
/  91 "kernel/clock/clock.c" 1
	movl eax, %esp;             movl edx, %eax;             mov %ax, %ss;             pop %gs;             pop %fs;             pop %es;             pop %ds;             popal;             add $8, %esp;             iret
/  0 "" 2
/NO_APP
	nop
	leave
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE1:
	.size	timer_callback, .-timer_callback
	.section	.rodata
	.align 4
.LC8:
	.string	"clock task executing at cpu: %u with id: %u %h"
	.text
	.globl	clock_task_entry_point
	.type	clock_task_entry_point, @function
clock_task_entry_point:
.LFB2:
	.cfi_startproc
	push	ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	mov	ebp, esp
	.cfi_def_cfa_register 5
	push	esi
	push	ebx
	.cfi_offset 6, -12
	.cfi_offset 3, -16
	sub	esp, 12
	push	OFFSET FLAT:lock
	call	acquire_spinlock
	add	esp, 16
	call	get_stack
	mov	esi, eax
	call	get_current_thread
	movzx	eax, WORD PTR [eax+80]
	movzx	ebx, ax
	sub	esp, 12
	push	0
	call	per_cpu_read
	add	esp, 16
	push	esi
	push	ebx
	push	eax
	push	OFFSET FLAT:.LC8
	call	printfln
	add	esp, 16
	sub	esp, 12
	push	OFFSET FLAT:lock
	call	release_spinlock
	add	esp, 16
	sub	esp, 8
	push	OFFSET FLAT:timer_callback
	push	64
	call	isr_register
	add	esp, 16
.L6:
	jmp	.L6
	.cfi_endproc
.LFE2:
	.size	clock_task_entry_point, .-clock_task_entry_point
	.local	x.2301
	.comm	x.2301,4,4
	.ident	"GCC: (GNU) 7.3.0"
