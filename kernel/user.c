// user mode test program
// ! This is a TEST file

int call_service(int num)
{
    asm ("movl %0, %%eax" : : "m"(num) : "eax");
    asm volatile("int $0x80");
}


int do_user()
{
    call_service(10);

    for(;;);

    call_service(20);

    return 0;
}