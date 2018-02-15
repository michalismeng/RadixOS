SOURCES := $(wildcard *.c)
LIBSOURCES := $(wildcard libk/*.c) $(wildcard libk/*.nsm)

ASSEMS 	:= $(wildcard *.s)
NASMS	:= $(wildcard *.nsm)		# .nsm files are compiled with nasm
OBJECTS := $(patsubst %.c, %.o, $(SOURCES)) $(patsubst %.s, %.o, $(ASSEMS)) $(patsubst %.nsm, %.o, $(NASMS)) $(patsubst %.c, %.o, $(LIBSOURCES))
INC 	:= include libk/include
INCLUDES := $(patsubst %, -I%, $(INC))
TARGET := objs

OBJ_DEPS := $(patsubst libk/%,%,$(OBJECTS))
OBJ_DEPS := $(patsubst %,$(TARGET)/%,$(OBJ_DEPS))

# rule to build the whole system
all: $(OBJECTS)
	$$elf_gcc -T linker.ld -nostdlib -lgcc $(OBJ_DEPS) -o radixOS.bin -O2


# rule to run the system
run: all
	grub-file --is-x86-multiboot radixOS.bin
	qemu-system-i386 -smp cores=2,threads=1,sockets=1 -m 128 -kernel radixOS.bin

run-uni: all
	grub-file --is-x86-multiboot radixOS.bin
	qemu-system-i386 -smp cores=2,threads=1,sockets=1 -m 128 -kernel radixOS.bin

# rule to compile c source file
%.o: %.c
	$$elf_gcc -c $< -o $(TARGET)/$(patsubst libk/%,%,$@) -std=gnu99 -ffreestanding -O2 -Wall -Wextra $(INCLUDES)

# rule to compile assembly file
%.o: %.s
	$$elf_as -c $< -o $(TARGET)/$@

# rule to compile nasm assembly file
%.o: %.nsm
	nasm -f elf32 $< -o $(TARGET)/$@

clean:
	rm -f radixOS.bin
	rm -rf objs/*
