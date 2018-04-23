CC := $$elf_gcc# 		path to cross compiler		(i686-elf-gcc)
AS := $$elf_as# 		path to cross assembler		(i686-elf-as)
AR := $$elf_ar#			path to cross archiver		(i686-elf-ar)	<--- This doesn't work when called by parent makefile. TODO: FIX
AR := /home/michalis/opt/cross/bin/i686-elf-ar#			path to cross archiver		(i686-elf-ar) 

# The current makefile configuration does not take into account include file dependencies!

SOURCES := $(wildcard *.c)

INC_DEST := include
INCLUDES := $(patsubst %, -I%, $(INC_DEST))

OBJECTS :=  $(SOURCES:.c=.o)
OBJ_DEST := objs
OBJ_DEPS := $(patsubst %,$(OBJ_DEST)/%,$(OBJECTS))
	
build: $(OBJ_DEPS) 
	$(AR) rcs libk.a $(OBJ_DEPS)

# rule to compile c source file
$(OBJ_DEST)/%.o: %.c
	$(CC) -c $< -o $@ -std=gnu99 -ffreestanding -O0 -Wall -Wextra $(INCLUDES)
	
clean:
	rm -rf objs/*
	rm -f libk.a