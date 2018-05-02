#include <multiboot.h>
#include <kernel_definitions.h>

extern void __kernel_main(multiboot_info_t* mbd, unsigned int magic);

extern uint8_t __kernel_init_end;
// reserve 32 KB of empty for the page directory 
uint8_t* dummy_space = 0x1500000;


void kernel_main(multiboot_info_t* mbd, unsigned int magic)
{
    char nums[] = "0123456789";
    char y[] = "DEADALLOC";
    char z[] = "DALLOCINIT";
    char* x = 0xB8000;

    // TODO: relocate mbd to 3GB
    // Map the actual kernel from kernel_start to 3GB

    uint32_t init_pages = ((uint32_t)&__kernel_init_end - (uint32_t)&__kernel_init_start) / PAGE_SIZE;
    if(((uint32_t)&__kernel_init_end - (uint32_t)&__kernel_init_start) % PAGE_SIZE != 0)
        init_pages++;

    // ceil division to get kernel pages
    uint32_t kernel_pages = (KERNEL_END - KERNEL_START) / PAGE_SIZE;
    if((KERNEL_END - KERNEL_START) % PAGE_SIZE != 0)
        kernel_pages++;

    // "allocate" address space
    uint32_t* page_directory = dummy_space;
    uint32_t dummy_space_index = 4096;

    for(uint32_t i = 0; i < 1024 * 32; i++)
        dummy_space[i] = 0;

    // printfln("init pages: %h %h %u", &__kernel_init_start, &__kernel_init_end, init_pages);
    // printfln("page directory allocated: %h", page_directory);

    // uint32_t sum = KERNEL_PHYSICAL_START;
    // if(sum == 0)
    //     x[0] = 'A';
    // for(int i = 0; sum > 0; i++)
    // {
    //     x[2*i] = nums[sum % 10];
    //     sum = sum / 10;
    // }

    // asm("hlt");

    for(uint32_t index = 0; index < kernel_pages; index++)
    {
        physical_addr pa = KERNEL_PHYSICAL_START + index * PAGE_SIZE;
        virtual_addr  va = KERNEL_START + index * PAGE_SIZE;

        uint32_t* entry_space;

        uint32_t table_index = va >> 22;
        uint32_t entry_index = (va >> 12) & 0x03FF;

        if((page_directory[table_index] & ~0xFFF) == 0)
        {
            entry_space = dummy_space + dummy_space_index;
            dummy_space_index += 4096;
            page_directory[table_index] = (uint32_t)entry_space | 3;
            // printfln("allocated page table at: %h with %h", entry_space, page_directory[table_index]);

            for(uint8_t* j = entry_space; j < entry_space + 4096; j++)
                *j = 0;
        }

        uint32_t* entry = page_directory[table_index] & ~0xFFF;
        entry[entry_index] = pa | 3;

        // printfln("wrote at entry: %h value %h", entry + entry_index, entry[entry_index]);

        if(dummy_space_index >= 32 * 1024)
        {
            for(int i = 0; i < 8; i++)
                x[2*i] = y[i];

            asm("hlt");
        }
        // map the pages
    }

    for(uint32_t index = 0; index < init_pages; index++)
    {
        physical_addr pa = (uint32_t)&__kernel_init_start + index * PAGE_SIZE;
        virtual_addr  va = pa;

        uint32_t* entry_space;

        uint32_t table_index = va >> 22;
        uint32_t entry_index = (va >> 12) & 0x03FF;

        if((page_directory[table_index] & ~0xFFF) == 0)
        {
            entry_space = dummy_space + dummy_space_index;
            dummy_space_index += 4096;
            page_directory[table_index] = (uint32_t)entry_space | 3;
            // printfln("allocated page table at: %h with %h", entry_space, page_directory[table_index]);

            for(uint8_t* j = entry_space; j < entry_space + 4096; j++)
                *j = 0;
        }

        uint32_t* entry = page_directory[table_index] & ~0xFFF;
        entry[entry_index] = pa | 3;

        // printfln("wrote at entry: %h value %h", entry + entry_index, entry[entry_index]);

        if(dummy_space_index >= 32 * 1024)
        {
            for(int i = 0; i < 8; i++)
                x[2*i] = z[i];

            asm("hlt");
        }
        // map the pages
    }

    // print page directory
    // for(int i = 0; i < 1024; i++)
    // {
    //     if((page_directory[i] & ~0xFFF) != 0)
    //     {
    //         printfln("table: %u at %h has:", i, page_directory[i] & ~0xFFF);
    //         for(int j = 0; j < 10; j++)
    //         {
    //             uint32_t* entry = page_directory[i] & ~0xFFF;
    //             printf("entry has: %h", page_directory[i]);
    //             if((*entry & ~0xFFF) != 0)
    //                 printfln("entry: %u at %h", j, *entry & ~0xFFF);
    //         }
    //     }
    // }

    uint32_t out;
    // setup current address space and enable paging
    asm ("movl %1, %%eax; movl %%eax, %%cr3; movl %%eax, %0":"=r"(out):"r"(page_directory):"%eax");

    // printfln("out: %h", out);
    asm ("movl %cr0, %eax; orl $0x80000001, %eax; movl %eax, %cr0");

    // asm("jmp __kernel_main");
    __kernel_main(0, 0);

    for(;;);
}