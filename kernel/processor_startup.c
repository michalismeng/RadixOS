#include <processor_startup.h>
#include <gst.h>
#include <lapic.h>
#include <ap_boot.h>
#include <screen.h>
#include <spinlock.h>
#include <mem_manager_phys.h>
#include <mem_manager_virt.h>
#include <kernel_definitions.h>
#include <debug.h>
#include <utility.h>

extern uint32_t lock;  // lock, used to test spinlock functions when printing

// private functions and data

spinlock_t ready = 0;

uint32_t get_stack();

// startup the processor connected to 'lapic_id' and set it to execute at address 'exec_base'
void processor_startup(uint32_t lapic_id, physical_addr exec_base)
{
	// implement intel protocol for processor boot 
		
	// send the INIT interrupt and wait for 100ms
	lapic_send_ipi(get_gst()->lapic_base, lapic_id, 0, LAPIC_DELIVERY_INIT, LAPIC_DESTINATION_PHYSICAL, LAPIC_DESTINATION_TARGET);
	lapic_sleep(100);

	// send the STARTUP interrupt and wait for 1ms
	lapic_send_ipi(get_gst()->lapic_base, lapic_id, exec_base >> 12, LAPIC_DELIVERY_SIPI, 0, 0);
    lapic_sleep(1);
    // at this line processor has started spinning
}

// this function is called from the assembled 'ap_boot.fasm' after starting the processor
// GS register should be set to the processor's local data area, prior to calling this function
void setup_processor()
{
	// use the BSP page directory
	virt_mem_switch_directory(get_gst()->BSP_dir);

	idtr_install(&get_gst()->idtr);

	uint32_t id = per_cpu_read(PER_CPU_OFFSET(id));
    printfln("processor %u is awake at stack %h", per_cpu_read(PER_CPU_OFFSET(id)), get_stack());

	lapic_enable(get_gst()->lapic_base);
	// calibrate the lapic timer of the AP
	lapic_calibrate_timer(get_gst()->lapic_base, 10, 64);

    // setup usermode kernel stack (one per processor) 
    tss_set_kernel_stack(&get_cpu_storage(id)->tss_entry, alloc_perm() + 4096, GDT_SS_ENTRY(id) * 8);

	// give the mark to the BSP to continue waking up processors
	release_lock(&ready);
	asm("sti");

	while(1)
	{
		acquire_lock(&lock);

		int tempX = cursorX, tempY = cursorY;
		SetPointer(0, SCREEN_HEIGHT - 2 * (id + 1));

		printf("time: %u %u", lapic_millis(), per_cpu_read(PER_CPU_OFFSET(id)));

		SetPointer(tempX, tempY);

		release_lock(&lock);

		for(int i = 0; i < 10000 + id * 1000; i++);
	}
}

// this function sets up higher level services like virtual memory, ipc... for the processor
void final_processor_setup()
{
	// here we do the final setup of all the processors
	// concerning virtual memory - lapic timers etc
}

// public functions

void startup_all_AP()
{
    // this is a hardcoded memory location required by the assembled 'ap_boot.fasm'
	memcpy(0x8000, ap_boot_bin, ap_boot_bin_len);       // copy the ap_boot code to the predefined memory address
	*(uint32_t*)0x8002 = setup_processor;               // target 'c' function address to be called by the assembler
	*(gdt_ptr_t*)0x8006 = get_gst()->gdtr;              // global descriptor table register address (processors share a common descriptor with different entries)

	// currently the stack for each processor is allocated using the physical memory manager 
    // startup all but the first (BSP) processors

	acquire_lock(&ready);
    for(uint32_t i = 1; i < get_gst()->processor_count; i++)
	{
		if(get_gst()->per_cpu_data_base[i].enabled == 0)
			continue;
		
		*(uint16_t*)0x800C = GDT_SS_ENTRY(i);                               // mark the gdt entry so that 'ap_boot.fasm' can set the SS register accordingly
		*(uint32_t*)0x800E = phys_mem_alloc_above_1mb() + 4096;				// reserve 4KB stack for each processor. must be identity mapped region

		processor_startup(get_gst()->per_cpu_data_base[i].id, 0x8000);

		// wait for the processor to gracefully boot 
		acquire_lock(&ready);
	}
}