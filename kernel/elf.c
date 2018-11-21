#include <elf.h>

#include <utility.h>
#include <mem_manager_virt.h>

void elf_print_metadata(elf32_ehdr_t* hdr)
{
    elf_print_header(hdr);

    printfln("program headers:");

    for(int i = 0; i < hdr->e_phnum; i++)
    {
        elf32_program_hdr_t* phdr = (uint8_t*)hdr + hdr->e_phoff + i * hdr->e_phentsize;
        elf_print_program_header(phdr);
    }
}


void elf_print_header(elf32_ehdr_t* hdr)
{
    printf("Magic: ");
    for(int i = 0; i < EI_NIDENT; i++)
        printf("%x ", *((char*)hdr + i));
    printfln("");

    printfln("Data: %s endian", hdr->e_endian ? "little" : "big");
    printfln("Version: %u %u", hdr->e_iversion, hdr->e_version);
    printfln("Type: %u", hdr->e_type);
    printfln("Machine: %u", hdr->e_machine);

    printfln("Entry point: %h", hdr->e_entry);
    printfln("Start of program headers: %u", hdr->e_phoff);
    printfln("Start of section headers: %u", hdr->e_shoff);

    printfln("Size of header: %u", hdr->e_ehsize);
    printfln("Size of program headers: %u", hdr->e_phentsize);
    printfln("Size of section headers: %u", hdr->e_shentsize);

    printfln("Number of program headers: %u", hdr->e_phnum);
    printfln("Number of section headers: %u", hdr->e_shnum);
}

void elf_print_program_header(elf32_program_hdr_t* hdr)
{
    switch(hdr->p_type)
    {
        case PT_LOAD:       printf("LOAD "); break;
        case PT_DYNAMIC:    printf("DYNAMIC "); break;
        case PT_INTERP:     printf("INTERP"); break;
        case PT_NOTE:       printf("NOTE"); break;
        default: printfln("unknown type"); return;
    }

    printf("%h %h %h %h %h ", hdr->p_offset, hdr->p_vaddr, hdr->p_paddr, hdr->p_filesz, hdr->p_memsz);

    printf("%s%s%s", hdr->p_flags & PF_EXEC ? "X " : "", hdr->p_flags & PF_WRITE ? "W " : "", hdr->p_flags & PF_READ ? "R " : "");

    printfln("%h", hdr->p_align);
}

error_t elf_load(elf32_ehdr_t* hdr)
{
    for(int i = 0; i < hdr->e_phnum; i++)
    {
        printfln("loading header");
        elf32_program_hdr_t* phdr = (uint8_t*)hdr + hdr->e_phoff + i * hdr->e_phentsize;
        if(elf_load_program_header(hdr, phdr) != ERROR_OK)
            return ERROR_OCCUR;
    }

    return ERROR_OK;
}

error_t elf_load_program_header(elf32_ehdr_t* hdr, elf32_program_hdr_t* phdr)
{
    if(!(phdr->p_type & PT_LOAD))
        return ERROR_OCCUR;

    uint32_t flags = I86_PTE_USER | I86_PTE_PRESENT;
    if(phdr->p_flags & PF_WRITE)
        flags |= I86_PTE_WRITABLE;

    virt_mem_alloc_page_f(phdr->p_vaddr, flags);
    memset(phdr->p_vaddr, 0, phdr->p_memsz);
    memcpy(phdr->p_vaddr, (uint32_t)hdr + phdr->p_offset, phdr->p_filesz);

    return ERROR_OK;
}