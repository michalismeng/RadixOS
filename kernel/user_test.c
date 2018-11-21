// user mode test program
// ! This is a TEST file

int data[100];

int x = 15, y = 20;

// void _start()
// {
//     do_user();
// }

int do_user()
{
    call_service(10);
    call_service(12);
    for(;;);

    call_service(10);

    int count = call_service(11);

    for(int i = 0; i < count; i++)
        call_service(12);

    call_service(13);

    // for(;;);

    call_service(20);
}

int call_service(int num)
{
    int ret = 15;
    asm ("movl %0, %%eax" : : "m"(num) : "eax");
    asm("int $0x80; movl %%eax, %0":"m="(ret)::"eax");
    return ret;
}

void do_user2()
{
    call_service(14);

    call_service(20);

    for(;;);
}