#include <pic.h>
#include <system.h>

void pic_disable()
{
    uint8_t cw = RECEIVE_ICW4 | INITIALIZATION;

    // send the first command requiring initialization and reception of icw 4

    outportb(PIC_PRIMARY_COMMAND_PORT, cw);
	outportb(PIC_SLAVE_COMMAND_PORT, cw);

    //send data concerning irq mapping (map them away)

    outportb(PIC_PRIMARY_DATA_PORT, 0xE0);
	outportb(PIC_SLAVE_DATA_PORT, 0xE8);

    // send data concerning the irq master slave connection lines

    outportb(PIC_PRIMARY_DATA_PORT, PRIMARY_IRQ_LINE_2);
	outportb(PIC_SLAVE_DATA_PORT, SLAVE_IRQ_LINE_2);

    // send the fourth data packet defining 80x86 mode of operation 

    outportb(PIC_PRIMARY_DATA_PORT, OPERATION_80x86);
	outportb(PIC_SLAVE_DATA_PORT, OPERATION_80x86);

    // mask all interrupts, effectively disabling the PIC

    outportb(PIC_PRIMARY_DATA_PORT, 0xFF);
	outportb(PIC_SLAVE_DATA_PORT, 0xFF);
}