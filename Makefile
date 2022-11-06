# assembler
ASM = /usr/bin/nasm
# compiler
CC = /usr/bin/gcc
# linker
LD = /usr/bin/ld
# grub iso creator
GRUB = /usr/bin/grub-mkrescue
# sources
SRC = src
LIBC = ./src/libc
ASM_SRC = $(SRC)/asm
# objects
OBJ = obj
ASM_OBJ = $(OBJ)/asm
CONFIG = ./config
OUT = out
INC = ./include
INCLUDE=-I$(INC)

MKDIR= mkdir -p
CP = cp -f
DEFINES=

# assembler flags
ASM_FLAGS = -f elf32
# compiler flags
CC_FLAGS = $(INCLUDE) $(DEFINES) -m32 -std=gnu99 -ffreestanding -Wall -Wextra
# linker flags, for linker add linker.ld file too
LD_FLAGS = -m elf_i386 -T $(CONFIG)/linker.ld -nostdlib

# target file to create in linking
TARGET=$(OUT)/MarsOS.bin

# iso file target to create
TARGET_ISO=$(OUT)/MarsOS.iso
ISO_DIR=$(OUT)/isodir

OBJECTS=$(ASM_OBJ)/entry.o $(ASM_OBJ)/load_gdt.o\
		$(ASM_OBJ)/load_idt.o $(ASM_OBJ)/exception.o $(ASM_OBJ)/irq.o $(ASM_OBJ)/bios32_call.o\
		$(OBJ)/io_ports.o $(OBJ)/vga.o\
		$(OBJ)/string.o $(OBJ)/console.o\
		$(OBJ)/gdt.o $(OBJ)/idt.o $(OBJ)/isr.o $(OBJ)/8259_pic.o $(OBJ)/ide.o $(OBJ)/tty.o\
		$(OBJ)/keyboard.o\
		$(OBJ)/kernel.o $(OBJ)/pmm.o $(OBJ)/kheap.o $(OBJ)/bios32.o $(OBJ)/vesa.o $(OBJ)/bitmap.o\
		$(OBJ)/fabs.o $(OBJ)/fabsf.o $(OBJ)/alpha.o $(OBJ)/iscntrl.o $(OBJ)/toupper.o $(OBJ)/isupper.o\
		$(OBJ)/tolower.o $(OBJ)/isdigit.o $(OBJ)/punct.o $(OBJ)/isgraph.o $(OBJ)/isalnum.o $(OBJ)/isprint.o $(OBJ)/toascii.o\
		$(OBJ)/isspace.o $(OBJ)/ispunct.o $(OBJ)/isalpha.o $(OBJ)/isblank.o $(OBJ)/nonspacing.o $(OBJ)/isxdigit.o $(OBJ)/islower.o $(OBJ)/isascii.o\
		$(OBJ)/imaxabs.o $(OBJ)/calloc.o $(OBJ)/atol.o $(OBJ)/imaxdiv.o $(OBJ)/strtoull.o $(OBJ)/div.o $(OBJ)/strtol.o $(OBJ)/strtoll.o $(OBJ)/heapsort.o\ 
		$(OBJ)/abs.o $(OBJ)/atoll.o $(OBJ)/qsort.o $(OBJ)/atof.o $(OBJ)/rand.o $(OBJ)/bsearch.o $(OBJ)/strtoul.o $(OBJ)/realloc.o $(OBJ)/labs.o $(OBJ)/atoi.o
		$(OBJ)/ldiv.o $(OBJ)/qsort_r.o $(OBJ)/heapsort_r.o $(OBJ)/llabs.o $(OBJ)/lldiv.o $(OBJ)/strtold.o

all: $(OBJECTS)
	@printf "[ linking... ]\n"
	$(LD) $(LD_FLAGS) -o $(TARGET) $(OBJECTS)
	grub-file --is-x86-multiboot $(TARGET)
	@printf "\n"
	@printf "[ building ISO... ]\n"
	$(MKDIR) $(ISO_DIR)/boot/grub
	$(CP) $(TARGET) $(ISO_DIR)/boot/
	$(CP) $(CONFIG)/grub.cfg $(ISO_DIR)/boot/grub/
	$(GRUB) -o $(TARGET_ISO) $(ISO_DIR)
	rm -f $(TARGET)

$(ASM_OBJ)/entry.o : $(ASM_SRC)/entry.asm
	@printf "[ $(ASM_SRC)/entry.asm ]\n"
	$(ASM) $(ASM_FLAGS) $(ASM_SRC)/entry.asm -o $(ASM_OBJ)/entry.o
	@printf "\n"

$(ASM_OBJ)/load_gdt.o : $(ASM_SRC)/load_gdt.asm
	@printf "[ $(ASM_SRC)/load_gdt.asm ]\n"
	$(ASM) $(ASM_FLAGS) $(ASM_SRC)/load_gdt.asm -o $(ASM_OBJ)/load_gdt.o
	@printf "\n"

$(ASM_OBJ)/load_idt.o : $(ASM_SRC)/load_idt.asm
	@printf "[ $(ASM_SRC)/load_idt.asm ]\n"
	$(ASM) $(ASM_FLAGS) $(ASM_SRC)/load_idt.asm -o $(ASM_OBJ)/load_idt.o
	@printf "\n"

$(ASM_OBJ)/exception.o : $(ASM_SRC)/exception.asm
	@printf "[ $(ASM_SRC)/exception.asm ]\n"
	$(ASM) $(ASM_FLAGS) $(ASM_SRC)/exception.asm -o $(ASM_OBJ)/exception.o
	@printf "\n"

$(ASM_OBJ)/irq.o : $(ASM_SRC)/irq.asm
	@printf "[ $(ASM_SRC)/irq.asm ]\n"
	$(ASM) $(ASM_FLAGS) $(ASM_SRC)/irq.asm -o $(ASM_OBJ)/irq.o
	@printf "\n"

$(ASM_OBJ)/bios32_call.o : $(ASM_SRC)/bios32_call.asm
	@printf "[ $(ASM_SRC)/bios32_call.asm ]\n"
	$(ASM) $(ASM_FLAGS) $(ASM_SRC)/bios32_call.asm -o $(ASM_OBJ)/bios32_call.o
	@printf "\n"

$(OBJ)/io_ports.o : $(SRC)/io_ports.c
	@printf "[ $(SRC)/io_ports.c ]\n"
	$(CC) $(CC_FLAGS) -c $(SRC)/io_ports.c -o $(OBJ)/io_ports.o
	@printf "\n"

$(OBJ)/vga.o : $(SRC)/vga.c
	@printf "[ $(SRC)/vga.c ]\n"
	$(CC) $(CC_FLAGS) -c $(SRC)/vga.c -o $(OBJ)/vga.o
	@printf "\n"

$(OBJ)/string.o : $(SRC)/string.c
	@printf "[ $(SRC)/string.c ]\n"
	$(CC) $(CC_FLAGS) -c $(SRC)/string.c -o $(OBJ)/string.o
	@printf "\n"

$(OBJ)/console.o : $(SRC)/console.c
	@printf "[ $(SRC)/console.c ]\n"
	$(CC) $(CC_FLAGS) -c $(SRC)/console.c -o $(OBJ)/console.o
	@printf "\n"

$(OBJ)/gdt.o : $(SRC)/gdt.c
	@printf "[ $(SRC)/gdt.c ]\n"
	$(CC) $(CC_FLAGS) -c $(SRC)/gdt.c -o $(OBJ)/gdt.o
	@printf "\n"

$(OBJ)/idt.o : $(SRC)/idt.c
	@printf "[ $(SRC)/idt.c ]\n"
	$(CC) $(CC_FLAGS) -c $(SRC)/idt.c -o $(OBJ)/idt.o
	@printf "\n"

$(OBJ)/isr.o : $(SRC)/isr.c
	@printf "[ $(SRC)/isr.c ]\n"
	$(CC) $(CC_FLAGS) -c $(SRC)/isr.c -o $(OBJ)/isr.o
	@printf "\n"

$(OBJ)/8259_pic.o : $(SRC)/8259_pic.c
	@printf "[ $(SRC)/8259_pic.c ]\n"
	$(CC) $(CC_FLAGS) -c $(SRC)/8259_pic.c -o $(OBJ)/8259_pic.o
	@printf "\n"

$(OBJ)/ide.o : $(SRC)/ide.c
	@printf "[ $(SRC)/ide.c ]\n"
	$(CC) $(CC_FLAGS) -c $(SRC)/ide.c -o $(OBJ)/ide.o
	@printf "\n"

$(OBJ)/keyboard.o : $(SRC)/keyboard.c
	@printf "[ $(SRC)/keyboard.c ]\n"
	$(CC) $(CC_FLAGS) -c $(SRC)/keyboard.c -o $(OBJ)/keyboard.o
	@printf "\n"

$(OBJ)/kernel.o : $(SRC)/kernel.c
	@printf "[ $(SRC)/kernel.c ]\n"
	$(CC) $(CC_FLAGS) -c $(SRC)/kernel.c -o $(OBJ)/kernel.o
	@printf "\n"

$(OBJ)/pmm.o : $(SRC)/pmm.c
	@printf "[ $(SRC)/pmm.c ]\n"
	$(CC) $(CC_FLAGS) -c $(SRC)/pmm.c -o $(OBJ)/pmm.o
	@printf "\n"

$(OBJ)/kheap.o : $(SRC)/kheap.c
	@printf "[ $(SRC)/kheap.c ]\n"
	$(CC) $(CC_FLAGS) -c $(SRC)/kheap.c -o $(OBJ)/kheap.o
	@printf "\n"

$(OBJ)/bios32.o : $(SRC)/bios32.c
	@printf "[ $(SRC)/bios32.c ]\n"
	$(CC) $(CC_FLAGS) -c $(SRC)/bios32.c -o $(OBJ)/bios32.o
	@printf "\n"

$(OBJ)/vesa.o : $(SRC)/vesa.c
	@printf "[ $(SRC)/vesa.c ]\n"
	$(CC) $(CC_FLAGS) -c $(SRC)/vesa.c -o $(OBJ)/vesa.o
	@printf "\n"

$(OBJ)/tty.o : $(SRC)/tty.c
	@printf "[ $(SRC)/tty.c ]\n"
	$(CC) $(CC_FLAGS) -c $(SRC)/tty.c -o $(OBJ)/tty.o
	@printf "\n"

$(OBJ)/bitmap.o : $(SRC)/bitmap.c
	@printf "[ $(SRC)/bitmap.c ]\n"
	$(CC) $(CC_FLAGS) -c $(SRC)/bitmap.c -o $(OBJ)/bitmap.o
	@printf "\n"

$(OBJ)/cluster.o : $(SRC)/fs/cluster.c
	@printf "[ $(SRC)/fs/cluster.c ]\n"
	$(CC) $(CC_FLAGS) -c $(SRC)/fs/cluster.c -o $(OBJ)/cluster.o
	@printf "\n"

$(OBJ)/fentry.o : $(SRC)/fs/fentry.c
	@printf "[ $(SRC)/fs/fentry.c ]\n"
	$(CC) $(CC_FLAGS) -c $(SRC)/fs/fentry.c -o $(OBJ)/fentry.o
	@printf "\n"

$(OBJ)/main.o : $(SRC)/fs/main.c
	@printf "[ $(SRC)/fs/main.c ]\n"
	$(CC) $(CC_FLAGS) -c $(SRC)/fs/main.c -o $(OBJ)/main.o
	@printf "\n"

$(OBJ)/part_info.o : $(SRC)/fs/part_info.c
	@printf "[ $(SRC)/fs/part_info.c ]\n"
	$(CC) $(CC_FLAGS) -c $(SRC)/fs/part_info.c -o $(OBJ)/part_info.o
	@printf "\n"

$(OBJ)/vfat.o : $(SRC)/fs/vfat.c
	@printf "[ $(SRC)/fs/vfat.c ]\n"
	$(CC) $(CC_FLAGS) -c $(SRC)/fs/vfat.c -o $(OBJ)/vfat.o
	@printf "\n"

$(OBJ)/cache.o : $(SRC)/fs/cache.c
	@printf "[ $(SRC)/fs/cache.c ]\n"
	$(CC) $(CC_FLAGS) -c $(SRC)/fs/cache.c -o $(OBJ)/cache.o
	@printf "\n"

$(OBJ)/hash.o : $(SRC)/fs/hash.c
	@printf "[ $(SRC)/fs/hash.c ]\n"
	$(CC) $(CC_FLAGS) -c $(SRC)/fs/hash.c -o $(OBJ)/hash.o
	@printf "\n"

$(OBJ)/fabs.o : $(LIBC)/math/fabs.c
	@printf "[ $(LIBC)/fs/hash.c ]\n"
    $(CC) $(CC_FLAGS) -c $(LIBC)/math/fabs.c -o $(OBJ)/fabs.o
    @printf "\n"

$(OBJ)/fabsf.o : $(LIBC)/math/fabsf.c
    $(CC) $(CC_FLAGS) -c $(LIBC)/math/fabsf.c -o $(OBJ)/fabsf.o

$(OBJ)/alpha.o : $(LIBC)/ctype/alpha.h
    $(CC) $(CC_FLAGS) -c $(LIBC)/ctype/alpha.h -o $(OBJ)/alpha.o


$(OBJ)/iscntrl.o : $(LIBC)/ctype/iscntrl.c
    $(CC) $(CC_FLAGS) -c $(LIBC)/ctype/iscntrl.c -o $(OBJ)/iscntrl.o


$(OBJ)/toupper.o : $(LIBC)/ctype/toupper.c
    $(CC) $(CC_FLAGS) -c $(LIBC)/ctype/toupper.c -o $(OBJ)/toupper.o


$(OBJ)/isupper.o : $(LIBC)/ctype/isupper.c
    $(CC) $(CC_FLAGS) -c $(LIBC)/ctype/isupper.c -o $(OBJ)/isupper.o


$(OBJ)/tolower.o : $(LIBC)/ctype/tolower.c
    $(CC) $(CC_FLAGS) -c $(LIBC)/ctype/tolower.c -o $(OBJ)/tolower.o


$(OBJ)/isdigit.o : $(LIBC)/ctype/isdigit.c
    $(CC) $(CC_FLAGS) -c $(LIBC)/ctype/isdigit.c -o $(OBJ)/isdigit.o


$(OBJ)/punct.o : $(LIBC)/ctype/punct.h
    $(CC) $(CC_FLAGS) -c $(LIBC)/ctype/punct.h -o $(OBJ)/punct.o


$(OBJ)/isgraph.o : $(LIBC)/ctype/isgraph.c
    $(CC) $(CC_FLAGS) -c $(LIBC)/ctype/isgraph.c -o $(OBJ)/isgraph.o


$(OBJ)/isalnum.o : $(LIBC)/ctype/isalnum.c
    $(CC) $(CC_FLAGS) -c $(LIBC)/ctype/isalnum.c -o $(OBJ)/isalnum.o


$(OBJ)/isprint.o : $(LIBC)/ctype/isprint.c
    $(CC) $(CC_FLAGS) -c $(LIBC)/ctype/isprint.c -o $(OBJ)/isprint.o


$(OBJ)/toascii.o : $(LIBC)/ctype/toascii.c
    $(CC) $(CC_FLAGS) -c $(LIBC)/ctype/toascii.c -o $(OBJ)/toascii.o


$(OBJ)/isspace.o : $(LIBC)/ctype/isspace.c
    $(CC) $(CC_FLAGS) -c $(LIBC)/ctype/isspace.c -o $(OBJ)/isspace.o


$(OBJ)/ispunct.o : $(LIBC)/ctype/ispunct.c
    $(CC) $(CC_FLAGS) -c $(LIBC)/ctype/ispunct.c -o $(OBJ)/ispunct.o


$(OBJ)/isalpha.o : $(LIBC)/ctype/isalpha.c
    $(CC) $(CC_FLAGS) -c $(LIBC)/ctype/isalpha.c -o $(OBJ)/isalpha.o


$(OBJ)/isblank.o : $(LIBC)/ctype/isblank.c
    $(CC) $(CC_FLAGS) -c $(LIBC)/ctype/isblank.c -o $(OBJ)/isblank.o


$(OBJ)/nonspacing.o : $(LIBC)/ctype/nonspacing.h
    $(CC) $(CC_FLAGS) -c $(LIBC)/ctype/nonspacing.h -o $(OBJ)/nonspacing.o


$(OBJ)/isxdigit.o : $(LIBC)/ctype/isxdigit.c
    $(CC) $(CC_FLAGS) -c $(LIBC)/ctype/isxdigit.c -o $(OBJ)/isxdigit.o


$(OBJ)/islower.o : $(LIBC)/ctype/islower.c
    $(CC) $(CC_FLAGS) -c $(LIBC)/ctype/islower.c -o $(OBJ)/islower.o


$(OBJ)/isascii.o : $(LIBC)/ctype/isascii.c
    $(CC) $(CC_FLAGS) -c $(LIBC)/ctype/isascii.c -o $(OBJ)/isascii.o

$(OBJ)/asctime_r.o : $(LIBC)/time/asctime_r.c
    $(CC) $(CC_FLAGS) -c $(LIBC)/time/asctime_r.c -o $(OBJ)/asctime_r.o


$(OBJ)/asctime.o : $(LIBC)/time/asctime.c
    $(CC) $(CC_FLAGS) -c $(LIBC)/time/asctime.c -o $(OBJ)/asctime.o

$(OBJ)/imaxabs.o : $(LIBC)/stdlib/imaxabs.c
    $(CC) $(CC_FLAGS) -c $(LIBC)/stdlib/imaxabs.c -o $(OBJ)/imaxabs.o


$(OBJ)/calloc.o : $(LIBC)/stdlib/calloc.c
    $(CC) $(CC_FLAGS) -c $(LIBC)/stdlib/calloc.c -o $(OBJ)/calloc.o


$(OBJ)/atol.o : $(LIBC)/stdlib/atol.c
    $(CC) $(CC_FLAGS) -c $(LIBC)/stdlib/atol.c -o $(OBJ)/atol.o


$(OBJ)/imaxdiv.o : $(LIBC)/stdlib/imaxdiv.c
    $(CC) $(CC_FLAGS) -c $(LIBC)/stdlib/imaxdiv.c -o $(OBJ)/imaxdiv.o


$(OBJ)/strtoull.o : $(LIBC)/stdlib/strtoull.c
    $(CC) $(CC_FLAGS) -c $(LIBC)/stdlib/strtoull.c -o $(OBJ)/strtoull.o


$(OBJ)/div.o : $(LIBC)/stdlib/div.c
    $(CC) $(CC_FLAGS) -c $(LIBC)/stdlib/div.c -o $(OBJ)/div.o


$(OBJ)/strtol.o : $(LIBC)/stdlib/strtol.c
    $(CC) $(CC_FLAGS) -c $(LIBC)/stdlib/strtol.c -o $(OBJ)/strtol.o


$(OBJ)/strtoll.o : $(LIBC)/stdlib/strtoll.c
    $(CC) $(CC_FLAGS) -c $(LIBC)/stdlib/strtoll.c -o $(OBJ)/strtoll.o


$(OBJ)/heapsort.o : $(LIBC)/stdlib/heapsort.c
    $(CC) $(CC_FLAGS) -c $(LIBC)/stdlib/heapsort.c -o $(OBJ)/heapsort.o


$(OBJ)/abs.o : $(LIBC)/stdlib/abs.c
    $(CC) $(CC_FLAGS) -c $(LIBC)/stdlib/abs.c -o $(OBJ)/abs.o


$(OBJ)/atoll.o : $(LIBC)/stdlib/atoll.c
    $(CC) $(CC_FLAGS) -c $(LIBC)/stdlib/atoll.c -o $(OBJ)/atoll.o


$(OBJ)/qsort.o : $(LIBC)/stdlib/qsort.c
    $(CC) $(CC_FLAGS) -c $(LIBC)/stdlib/qsort.c -o $(OBJ)/qsort.o


$(OBJ)/atof.o : $(LIBC)/stdlib/atof.c
    $(CC) $(CC_FLAGS) -c $(LIBC)/stdlib/atof.c -o $(OBJ)/atof.o


$(OBJ)/rand.o : $(LIBC)/stdlib/rand.c
    $(CC) $(CC_FLAGS) -c $(LIBC)/stdlib/rand.c -o $(OBJ)/rand.o


$(OBJ)/bsearch.o : $(LIBC)/stdlib/bsearch.c
    $(CC) $(CC_FLAGS) -c $(LIBC)/stdlib/bsearch.c -o $(OBJ)/bsearch.o


$(OBJ)/strtoul.o : $(LIBC)/stdlib/strtoul.c
    $(CC) $(CC_FLAGS) -c $(LIBC)/stdlib/strtoul.c -o $(OBJ)/strtoul.o


$(OBJ)/realloc.o : $(LIBC)/stdlib/realloc.c
    $(CC) $(CC_FLAGS) -c $(LIBC)/stdlib/realloc.c -o $(OBJ)/realloc.o


$(OBJ)/labs.o : $(LIBC)/stdlib/labs.c
    $(CC) $(CC_FLAGS) -c $(LIBC)/stdlib/labs.c -o $(OBJ)/labs.o


$(OBJ)/atoi.o : $(LIBC)/stdlib/atoi.c
    $(CC) $(CC_FLAGS) -c $(LIBC)/stdlib/atoi.c -o $(OBJ)/atoi.o


$(OBJ)/ldiv.o : $(LIBC)/stdlib/ldiv.c
    $(CC) $(CC_FLAGS) -c $(LIBC)/stdlib/ldiv.c -o $(OBJ)/ldiv.o


$(OBJ)/qsort_r.o : $(LIBC)/stdlib/qsort_r.c
    $(CC) $(CC_FLAGS) -c $(LIBC)/stdlib/qsort_r.c -o $(OBJ)/qsort_r.o


$(OBJ)/heapsort_r.o : $(LIBC)/stdlib/heapsort_r.c
    $(CC) $(CC_FLAGS) -c $(LIBC)/stdlib/heapsort_r.c -o $(OBJ)/heapsort_r.o


$(OBJ)/llabs.o : $(LIBC)/stdlib/llabs.c
    $(CC) $(CC_FLAGS) -c $(LIBC)/stdlib/llabs.c -o $(OBJ)/llabs.o


$(OBJ)/lldiv.o : $(LIBC)/stdlib/lldiv.c
    $(CC) $(CC_FLAGS) -c $(LIBC)/stdlib/lldiv.c -o $(OBJ)/lldiv.o


$(OBJ)/strtold.o : $(LIBC)/stdlib/strtold.c
    $(CC) $(CC_FLAGS) -c $(LIBC)/stdlib/strtold.c -o $(OBJ)/strtold.o




clean:
	rm -f $(OBJ)/*.o
	rm -f $(ASM_OBJ)/*.o
	rm -rf $(OUT)/*
