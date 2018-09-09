// user mode test program
// This is a TEST file

void call_service(int num)
{
    // asm volatile ("mov %0, %%edx" : "=r" (num));
    asm ("movl %0, %%eax" : : "b"(num) : "eax");
    asm volatile("int $0x80");
}


int do_user()
{
    int x = 0;
    while(x < 10)
        x++;

    call_service(20);

    for(;;);

    return x;
}