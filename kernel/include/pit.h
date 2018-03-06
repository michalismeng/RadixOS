#ifndef PIT_H_20022018
#define PIT_H_20022018

#include <stdint.h>

#define TERMINAL_COUNT			0 << 1
#define PROG_ONESHOT			1 << 1
#define RATE_GENERATOR			2 << 1	// used for the timer
#define SQR_WAVE_GENERATOR		3 << 1	// used for the speaker
#define SOFT_TRIGGER			4 << 1
#define HARD_TRIGGER			5 << 1

#define LATCH					0 << 4
#define LSB_ONLY				1 << 4		// least significand byte
#define MSB_ONLY				2 << 4		// most significand byte only
#define LSB_THEN_MSB			3 << 4

#define COUNTER_0				0 << 6
#define COUNTER_1				1 << 6
#define COUNTER_2				2 << 6

#define CHANNEL_0               0x40
#define COMMAND                 0x43

// initializes the PIT at the given frequency and mode (one time or periodic)
void pit_timer_init(uint32_t _frequency, int one_time);

// returns the current count register value of the pit
volatile uint32_t pit_read_count();

// spins 'time' milliseconds have passed, based on the PIT count variable
void pit_sleep(uint32_t time);

#endif