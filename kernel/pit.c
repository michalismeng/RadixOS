#include <pit.h>

uint32_t pit_frequency;
extern volatile uint32_t pit_count;


void pit_timer_init(uint32_t _frequency)
{
	pit_frequency = _frequency;

	// frequency must be such that count fits in a 16-bit variable.
	uint16_t count = 1193180 / _frequency;

	//initialization command byte
	uint8_t cw = RATE_GENERATOR | LSB_THEN_MSB | COUNTER_0;

	outportb(0x43, cw);

	uint8_t l = (uint8_t)(count & 0x00FF);
	uint8_t h = (uint8_t)((count >> 8) & 0x00FF);

	outportb(0x40, l);	// send the least significand byte first
	outportb(0x40, h);	// then send the most.
	pit_count = 0;
}

volatile uint32_t millis()
{
	return (1000 * pit_count) / pit_frequency;
}

void pit_sleep(uint32_t time)
{
    volatile uint32_t cur_time = millis();

    while(millis() - cur_time < time);
}