#include <clock/clock.h>
#include <debug.h>
#include <isr.h>
#include <per_cpu_data.h>
#include <gst.h>
#include <screen.h>
#include <lapic.h>

#include <ipc/ipc.h>

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
    printfln("clock task executing at cpu: %u with id: %u %h", get_cpu_id, get_current_thread()->tid, get_current_thread());
	release_spinlock(&lock);

    //! send message example
    // if(cpu_is_bsp)
    // {
    //     for(int i = 0; i < 20000000; i++);

    //     message_t msg;
    //     msg.src = get_current_thread()->mailbox->mid;
    //     msg.dst = get_current_thread()->mailbox->mid + 1;

    //     msg.func = 15;
    //     memcpy(&msg.payload.custom, "hello other thread", 19);
    //     printfln("sending message to: %u from %u", msg.dst, msg.src);
    //     send(&msg);
    //     printfln("mail sent");
    // }
    // else
    // {

    //     // message_t msg;
    //     // printfln("start receiving");
    //     // receive(get_current_thread()->mailbox, &msg);

    //     // acquire_spinlock(&lock);
    //     // printfln("received message from: %u to %u\nfunc: %u payload: %c", msg.src, msg.dst, msg.func, msg.payload.custom[0]);
    //     // release_spinlock(&lock);
    // }

    // if(cpu_is_bsp)
    // {
    //     for(int i = 0; i < 20000000; i++);

    //     message_t msg;
    //     msg.src = 0;
    //     msg.dst = 1;

    //     msg.func = 1;

    //     send(&msg);
    //     printfln("mail sent");
    //     msg.func = 2;
    //     send(&msg);
    //     printfln("mail sent");
    // }

    message_t msg;
    msg.src = msg.dst = get_cpu_id;
    msg.func = 10;
    send(&msg);

    printfln("here");
    // replace the timer callback with our own
    isr_register(64, timer_callback);


    while(1)
	{
		acquire_spinlock(&lock);

		int tempX = cursorX, tempY = cursorY;
		SetPointer(0, SCREEN_HEIGHT - 2 * (get_cpu_id + 1));

        time_print(&get_gst()->system_time);

		SetPointer(tempX, tempY);

		release_spinlock(&lock);

		for(int i = 0; i < 1000000; i++);		// do some random sleep to allow the other processor to acquire the lock
	}

    // TODO: enter in a message receiving state

    for(;;);
}