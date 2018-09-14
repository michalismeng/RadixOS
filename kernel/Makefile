CC := $$elf_gcc# 		path to cross compiler		(i686-elf-gcc)
AS := $$elf_as# 		path to cross assembler		(i686-elf-as)

# The current makefile configuration does not take into account include file dependencies!

SOURCES := $(wildcard *.c)\
		   $(wildcard clock/*.c)
ASSEMS 	:= $(wildcard *.s)			# .s files are compiled with gcc assembler
NASMS	:= $(wildcard *.asm)		# .asm files are compiled with nasm

INC_DEST 	:= include libk/include
INCLUDES := $(patsubst %, -I%, $(INC_DEST))

OBJECTS :=  $(SOURCES:.c=.o) $(ASSEMS:.s=.o) $(NASMS:.asm=.o)
OBJ_DEST := objs
OBJ_DEPS := $(patsubst %,$(OBJ_DEST)/%,$(OBJECTS))

.PHONY: clean

# rule to build the whole system
all: ap_boot $(OBJ_DEPS)
	cd libk && make
	$(CC) -T linker.ld -nostdlib -lgcc $(OBJ_DEPS) -o radixOS.bin -O0 -fno-stack-protector -Llibk -l:libk.a

	echo BUILD SUCESS

# rule to run the system in the default dual-core configuration
run: all
	grub-file --is-x86-multiboot radixOS.bin
	qemu-system-i386 -smp cores=2,threads=1,sockets=1 -m 128 -kernel radixOS.bin

debug: all
	grub-file --is-x86-multiboot radixOS.bin
	qemu-system-i386 -s -S -smp cores=2,threads=1,sockets=1 -m 128 -kernel radixOS.bin

# rule to run the system in the quad-core configuration
run-quad: all
	grub-file --is-x86-multiboot radixOS.bin
	qemu-system-i386 -smp cores=4,threads=1,sockets=1 -m 128 -kernel radixOS.bin

# rule to run the system in uniprocessor mode
run-uni: all
	grub-file --is-x86-multiboot radixOS.bin
	qemu-system-i386 -m 128 -kernel radixOS.bin

# rule to compile the processor boot code into a flat binary and turn it into a c-style array
ap_boot: ap_boot.fasm
	nasm -f bin $< -o ap_boot.bin
	xxd -i ap_boot.bin > include/ap_boot.h
	rm ap_boot.bin

# rule to compile c source file
$(OBJ_DEST)/%.o: %.c
	$(CC) -c $< -o $@ -std=gnu99 -ffreestanding -O0 -Wall -Wextra $(INCLUDES) -fno-stack-protector -Llibk -l:libk.a

# rule to compile assembly file
$(OBJ_DEST)/%.o: %.s
	$(AS) -c $< -o $@

# rule to compile nasm assembly file
$(OBJ_DEST)/%.o: %.asm
	nasm -f elf32 $< -o $@

clean:
	rm -f radixOS.bin
	# remove all files but keep folder structure
	find objs -type f -exec rm {} +
	rm include/ap_boot.h
