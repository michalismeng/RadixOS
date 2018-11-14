#include <debug.h>
#include <isr.h>
#include <per_cpu_data.h>
#include <gst.h>
#include <screen.h>

extern uint32_t lock;

static void update_system_time()
{
    ktime_t* time = &get_gst()->system_time;
    time->msec += 10;

    // millisecond overflow
    if(time->msec >= 1000)
    {
        time->msec = 0;
        time->sec++;

        // second overflow
        if(time->sec >= 60)
        {
            time->sec = 0;
            time->min++;

            // minute overflow
            if(time->min == 60)
            {
                time->min = 0;
                time->hour++;

                // hour overflow
                if(time->hour == 24)
                {
                    time->hour = 0;
                    time->day++;
                    time->weekday++;        // TODO: continue these two nasty overflows
                }
            }
        }
    }
}

static int32_t timer_callback(trap_frame_t* regs)
{
    // increment running time for the cpu
    per_cpu_write(PER_CPU_OFFSET(lapic_count), per_cpu_read(PER_CPU_OFFSET(lapic_count)) + 1);

    // only the BSP should update the system time
    if(cpu_is_bsp)
        update_system_time();
}

void clock_task_entry_point()
{
	acquire_spinlock(&lock);    
    printfln("clock task executing at cpu: %u", get_cpu_id);
	release_spinlock(&lock);

    // replace the timer callback with our own
    isr_register(64, timer_callback);

    if(cpu_is_bsp)
        scheduler_block_running_thread();

    while(1)
	{
		acquire_spinlock(&lock);

		int tempX = cursorX, tempY = cursorY;
		SetPointer(0, SCREEN_HEIGHT - 2 * (get_cpu_id + 1));

        time_print(&get_gst()->system_time);

		SetPointer(tempX, tempY);

		release_spinlock(&lock);

		for(int i = 0; i < 10000; i++);		// do some random sleep to allow the other processor to acquire the lock
	}

    // TODO: enter in a message receiving state

    for(;;);
}