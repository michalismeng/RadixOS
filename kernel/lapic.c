#include <lapic.h>
#include <system.h>
#include <pit.h>
#include <per_cpu_data.h>
#include <debug.h>

void lapic_enable(physical_addr base_addr)
{
    volatile uint32_t val = reg_readl(base_addr, 0xF0);

    // enable the lapic only if not already enabled
    if(val & (1 << 8) == 0)
    {
        val |= (1 << 8);

        // setup the spurious interrupt vector register
        reg_writel(base_addr, 0xF0, val);
    }   
}

uint32_t lapic_get_id(physical_addr base_addr)
{
    return reg_readl(base_addr, LAPIC_ID);
}

void lapic_send_eoi(physical_addr base_addr)
{
    // write the value of 0 to the EOI register
    reg_writel(base_addr, LAPIC_EOI, 1);
}

void lapic_send_ipi(physical_addr base_addr, uint8_t target_id, uint8_t target_vector, uint32_t delivery_mode, uint32_t destination_mode, uint32_t destination_type)
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

	low |= (destination_type << 18) & LAPIC_ICR_DESTINATION_TYPE;

    reg_writel(base_addr, LAPIC_ICR_LOW, low);
}

void lapic_calibrate_timer(physical_addr base_addr, uint32_t target_period, uint8_t irq_vector)
{
    // save the requested period
    per_cpu_write(PER_CPU_OFFSET(lapic_period), target_period);
    per_cpu_write(PER_CPU_OFFSET(lapic_count), 0);

    reg_writel(base_addr, LAPIC_TIMER_DIV, 0x3);                            // set the divider register to 16

    // prepare pit for one shot mode (1ms period)
	pit_timer_init(1000, 1);    

    reg_writel(base_addr, LAPIC_TIMER_INIT, 0xFFFFFFFF);                    // set counter and implicitly one-shot mode

    while(pit_read_count() > 0);                                            // sleep for 10ms

    reg_writel(base_addr, LAPIC_TIMER_LVT, LAPIC_TIMER_DISABLE);

    uint32_t ticks_in_1_ms = 0xFFFFFFFF - reg_readl(base_addr, LAPIC_TIMER_CUR);
    printfln("found %u ticks in 1 ms", ticks_in_1_ms);
    printfln("found %u ticks in %u ms", ticks_in_1_ms * target_period, target_period);

    // the PIT is not useful anymore
    // pit_timer_init(1000, 0);

    // start APIC timer in periodic mode
    reg_writel(base_addr, LAPIC_TIMER_DIV, 0x3);
    reg_writel(base_addr, LAPIC_TIMER_LVT, irq_vector | LAPIC_TIMER_PERIODIC);
    reg_writel(base_addr, LAPIC_TIMER_INIT, ticks_in_1_ms * target_period);       // get the ticks in 'target_period' ms
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