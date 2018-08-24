#include <multiboot.h>
#include <kernel_definitions.h>

// The kernel initializer is executed after bootloading and is responsible for setting up virtual memory
// then it passes control to the kernel.

// The kernel is mapped at 3 GB and executed.

// libk cannot be used since it is mapped at the higher memory (3 GB)

// kernel entry point
extern void kernel_entry(multiboot_info_t* mbd);

// reserve 32 KB of empty for the page directory 
uint8_t* dummy_space = 0x80000;

void print_to_screen(char* msg, uint32_t length)
{
    char* vidmem = 0xB8000;

    for(int i = 0; i < length; i++)
    {
        vidmem[2*i] = msg[i];
        vidmem[2*i + 1] = 0x0F;
    }
}

uint32_t map_page(uint32_t* page_directory, physical_addr pa, virtual_addr va)
{
    static uint32_t dummy_space_index = 4096;

    uint32_t table_index = va >> 22;
    uint32_t entry_index = (va >> 12) & 0x03FF;

    if((page_directory[table_index] & ~0xFFF) == 0)     // page table does not exist
    {
        uint32_t* entry_space = dummy_space + dummy_space_index;
        dummy_space_index += 4096;

        page_directory[table_index] = (uint32_t)entry_space | 3;        // set the physical frame of the table and its presence flags

        for(uint8_t* j = entry_space; j < entry_space + 4096; j++)      // clear all table entries
            *j = 0;
    }

    uint32_t* entry = page_directory[table_index] & ~0xFFF;             // get the page entry physical address
    entry[entry_index] = pa | 3;                                        // set up the mapping between virtual and physical addresses

    return dummy_space_index;
}

void initializer_main(multiboot_info_t* mbd, unsigned int magic)
{
    // kernel initializer required pages
    uint32_t init_pages = ((uint32_t)&__kernel_init_end - (uint32_t)&__kernel_init_start) / PAGE_SIZE;
    if(((uint32_t)&__kernel_init_end - (uint32_t)&__kernel_init_start) % PAGE_SIZE != 0)
        init_pages++;

    // kernel required pages
    uint32_t kernel_pages = (KERNEL_END - KERNEL_START) / PAGE_SIZE;
    if((KERNEL_END - KERNEL_START) % PAGE_SIZE != 0)
        kernel_pages++;

    // "allocate" address space
    uint32_t* page_directory = dummy_space;

    // assume 32KB dummy space
    for(uint32_t i = 0; i < 1024 * 32; i++)
        dummy_space[i] = 0;

    // setup kernel virtual pages
    for(uint32_t index = 0; index < kernel_pages; index++)
    {
        physical_addr pa = KERNEL_PHYSICAL_START + index * PAGE_SIZE;
        virtual_addr  va = KERNEL_START + index * PAGE_SIZE;

        uint32_t current_index = map_page(page_directory, pa, va);

        if(current_index >= 32 * 1024)
        {
            char msg[] = "kernel is too large to fit in 32 KB";
            print_to_screen(msg, sizeof(msg));
            asm("hlt");
        }
    }

    // setup kernel initializer virtual pages as identity
    for(uint32_t index = 0; index < init_pages; index++)
    {
        physical_addr pa = (uint32_t)&__kernel_init_start + index * PAGE_SIZE;
        virtual_addr  va = pa;

        uint32_t current_index = map_page(page_directory, pa, va);

        if(current_index >= 32 * 1024)
        {
            char msg[] = "kernel initializer is too large to fit in 32 KB";
            print_to_screen(msg, sizeof(msg));

            asm("hlt");
        }
        // map the pages
    }

    // identity map the first MB to be able to print to the screen after enabling paging
    for(uint32_t index = 0; index < 256; index++)
    {
        physical_addr pa = index * PAGE_SIZE;
        virtual_addr  va = pa;

        uint32_t current_index = map_page(page_directory, pa, va);

        if(current_index >= 32 * 1024)
        {
            char msg[] = "cannot identity map 1st MB";
            print_to_screen(msg, sizeof(msg));

            asm("hlt");
        }
    }

    // setup current address space and enable paging
    asm ("movl %0, %%eax; movl %%eax, %%cr3;"::"r"(page_directory):"%eax");
    asm ("movl %cr0, %eax; orl $0x80000001, %eax; movl %eax, %cr0");

    kernel_entry(mbd);

    for(;;);
}