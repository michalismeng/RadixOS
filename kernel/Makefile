CC := $$elf_gcc# 		path to cross compiler		(i686-elf-gcc)
AS := $$elf_as# 		path to cross assembler		(i686-elf-as)

# The current makefile configuration does not take into account include file dependencies!

SOURCES := $(wildcard *.c)
ASSEMS 	:= $(wildcard *.s)
NASMS	:= $(wildcard *.asm)		# .asm files are compiled with nasm
LIBSOURCES := $(wildcard libk/*.c) $(wildcard libk/*.asm)

OBJECTS :=  $(SOURCES:.c=.o) $(ASSEMS:.s=.o) $(NASMS:.asm=.o)  $(LIBSOURCES:.c=.o)
INC 	:= include libk/include
INCLUDES := $(patsubst %, -I%, $(INC))
TARGET_OBJS := objs

OBJ_DEPS := $(patsubst libk/%,%,$(OBJECTS))
OBJ_DEPS := $(patsubst %,$(TARGET_OBJS)/%,$(OBJ_DEPS))

.PHONY: clean

# rule to build the whole system
all: $(OBJ_DEPS)
	$(CC) -T linker.ld -nostdlib -lgcc $(OBJ_DEPS) -o radixOS.bin -O0 -fno-stack-protector

# rule to run the system
run: all
	grub-file --is-x86-multiboot radixOS.bin
	qemu-system-i386 -smp cores=2,threads=1,sockets=1 -m 128 -kernel radixOS.bin

run-uni: all
	grub-file --is-x86-multiboot radixOS.bin
	qemu-system-i386 -m 128 -kernel radixOS.bin

# compile the processor boot code into flat binary and turn it into c-style array
ap_boot: ap_boot.fasm
	nasm -f bin $< -o ap_boot.bin
	xxd -i ap_boot.bin > include/ap_boot.h
	rm ap_boot.bin

# rule to compile c source file
$(TARGET_OBJS)/%.o: %.c
	$(CC) -c $< -o $@ -std=gnu99 -ffreestanding -O0 -Wall -Wextra $(INCLUDES) -fno-stack-protector

# rule to compile c source file of the kernel library
$(TARGET_OBJS)/%.o: libk/%.c
	$(CC) -c $< -o $(patsubst libk/%,%,$@) -std=gnu99 -ffreestanding -O2 -Wall -Wextra $(INCLUDES) -fno-stack-protector

# rule to compile assembly file
$(TARGET_OBJS)/%.o: %.s
	$(AS) -c $< -o $@

# rule to compile nasm assembly file
$(TARGET_OBJS)/%.o: %.asm
	nasm -f elf32 $< -o $@

clean:
	rm -f radixOS.bin
	rm -rf objs/*
	rm include/ap_boot.h
