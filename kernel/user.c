// user mode test program
// This is a TEST file

void call_service(int num)
{
    asm ("movl %0, %%ebx" : : "m"(num) : "ebx");
    asm ("mov %esp, %ecx");
    asm("mov %gs, %eax");
    asm volatile("int $0x80");
}


int do_user()
{
    int x = 0;
    while(x < 10)
        x++;

    asm ("movl $5, %eax; mov %eax, %gs");
    call_service(20);

    call_service(20);

    for(;;);

    return x;
}