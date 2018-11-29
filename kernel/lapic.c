#include <lapic.h>
#include <system.h>
#include <pit.h>
#include <per_cpu_data.h>
#include <debug.h>
#include <isr.h>
#include <mem_manager_virt.h>

// void lapic_error_callback(iregisters_t* regs)
// {
//     printfln("lapic error occured");
//     PANIC("");
// }

void lapic_enable(physical_addr_t base_addr)
{
    if(!virt_mem_is_page_present((uint32_t)base_addr & (~0xfff)))
        virt_mem_map_page(virt_mem_get_self_recursive_table(), (uint32_t)base_addr & (~0xfff), (uint32_t)base_addr & (~0xfff), VIRT_MEM_DEFAULT_PTE_FLAGS);

    volatile uint32_t val = reg_readl(base_addr, LAPIC_SPURIOUS_INTERRUPT);

    // enable the lapic only if not already enabled
    // printfln("processor 1 starts lapic");
    if(( val & (1 << 8) ) == 0)
    {
        val |= (1 << 8);

        // setup the spurious interrupt vector register
        reg_writel(base_addr, LAPIC_SPURIOUS_INTERRUPT, val | 0x100);
        // reg_writel(base_addr, LAPIC_ERROR_LVT, 65 | LAPIC_DELIVERY_FIXED);
    }   
} 

uint32_t lapic_get_id(physical_addr_t base_addr)
{
    return reg_readl(base_addr, LAPIC_ID);
}

void lapic_send_eoi(physical_addr_t base_addr)
{
    // write the value of 0 to the EOI register
    reg_writel(base_addr, LAPIC_EOI, 1);
}

void lapic_send_ipi_std(physical_addr_t base_addr, uint8_t target_id, uint8_t target_vector)
{
    lapic_send_ipi(base_addr, target_id, target_vector, LAPIC_DELIVERY_FIXED, LAPIC_DESTINATION_PHYSICAL, LAPIC_DESTINATION_TARGET);
}

void lapic_send_ipi_to_others(physical_addr_t base_addr, uint8_t target_vector)
{
    lapic_send_ipi(base_addr, 0, target_vector, LAPIC_DELIVERY_FIXED, LAPIC_DESTINATION_ALL_BUT_SELF, LAPIC_DESTINATION_TARGET);
}

void lapic_send_ipi(physical_addr_t base_addr, uint8_t target_id, uint8_t target_vector, uint32_t delivery_mode, uint32_t destination_mode, uint32_t destination_shorthand)
{
    // wait until there are no pending IPIs
    while(reg_readl(base_addr, LAPIC_ICR_LOW) & LAPIC_ICR_PENDING);

    // set the high ICR at first, as writing to the low one triggers the IPI
    reg_writel(base_addr, LAPIC_ICR_HIGH, (target_id & 0x0F) << 24);

    uint32_t low = target_vector;
    low |= (delivery_mode << 8) & LAPIC_ICR_DELIVERY;
    low |= (destination_mode << 11) & LAPIC_ICR_DESTINATION;

    if(delivery_mode == LAPIC_DELIVERY_INIT)
    {
        low &= ~(1 << 14);
        low &= ~(1 << 15);
    }
    else
        low |= (1 << 14);

	low |= destination_shorthand;

    reg_writel(base_addr, LAPIC_ICR_LOW, low);
}

void lapic_timer_callback(trap_frame_t* regs)
{
    // increment the cpu-local lapic counter
    per_cpu_write(PER_CPU_OFFSET(lapic_count), per_cpu_read(PER_CPU_OFFSET(lapic_count)) + 1);
}

void lapic_calibrate_timer(physical_addr_t base_addr, uint32_t target_period, uint8_t irq_vector)
{
    // save the requested period
    per_cpu_write(PER_CPU_OFFSET(lapic_period), target_period);
    per_cpu_write(PER_CPU_OFFSET(lapic_count), 0);

    reg_writel(base_addr, LAPIC_TIMER_DIV, 0x3);                            // set the divider register to 16

    uint32_t ticks_in_1_ms = 0;
    uint32_t tries = 5;

    do 
    {
        // prepare pit for one shot mode (1ms period)
        pit_timer_init(1000, 1);    

        reg_writel(base_addr, LAPIC_TIMER_INIT, 0xFFFFFFFF);                    // set counter and implicitly one-shot mode

        while(pit_read_count() > 0);                                            // sleep for 1ms

        reg_writel(base_addr, LAPIC_TIMER_LVT, LAPIC_TIMER_DISABLE);

        ticks_in_1_ms = 0xFFFFFFFF - reg_readl(base_addr, LAPIC_TIMER_CUR);
        tries--;
    } while(ticks_in_1_ms > 200000 && tries > 0);                             // typical value is around 60k => if something went wrong retry. 

    if(tries == 0)
        PANIC("Could not calibrate local APIC timer");

    printfln("found %u ticks in 1 ms -- tried: %u times", ticks_in_1_ms, 5 - tries);

    isr_register(irq_vector, lapic_timer_callback);

    // start APIC timer in periodic mode
    reg_writel(base_addr, LAPIC_TIMER_DIV, 0x3);                                    // set the divider register to 16
    reg_writel(base_addr, LAPIC_TIMER_LVT, irq_vector | LAPIC_TIMER_PERIODIC);      // set the timer local vector to the given one 
    reg_writel(base_addr, LAPIC_TIMER_INIT, ticks_in_1_ms * target_period);         // set the ticks in 'target_period' ms
}

volatile uint32_t lapic_millis()
{
    return per_cpu_read(PER_CPU_OFFSET(lapic_period)) * per_cpu_read(PER_CPU_OFFSET(lapic_count));
}

void lapic_sleep(uint32_t time)
{
    volatile uint32_t cur_time = lapic_millis();

    while(lapic_millis() - cur_time < time);
}