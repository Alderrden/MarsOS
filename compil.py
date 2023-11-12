import os
import sys
from termcolor import colored, cprint
import time

class compil(object):
    """docstring for compil"""
    def __init__(self):
        cprint("===================================================================================", "yellow")
        cprint("=                                MarsOS Compilator                                =", "yellow")
        cprint("===================================================================================", "yellow")
        print("\n")
        cprint("-= Remove old objects =-", "blue")
        os.system("rm -f ./obj/*.o")
        os.system("rm -rf ./iso/*")
        os.system("rm ext2.img")
        self.compil_asm()

    def compil_asm(self):
        cprint("-= Compile Assembler =-", "blue")
        for root, dirs, files in os.walk("./asm"):  
            for asm in files:
                command_asm = "nasm -f elf32 " + "./asm/" + asm + " -o ./obj/" + asm.replace(".asm", ".o")
                print(command_asm)
                os.system(command_asm)
        self.compil_c()

    def compil_c(self):
        cprint("-= Compile C code =-", "blue")
        for root, dirs, files in os.walk("./src"):  
            for src in files:
                command_c = "/home/alderrden/gcc-x86/bin/x86_64-elf-gcc -I./include  -m32 -std=gnu99 -ffreestanding -Wall -Wextra -c ./src/" + src + " -o ./obj/" + src.replace(".c", ".o")
                print(command_c)
                os.system(command_c)
        cprint("-= Compile kernel =-", "blue")
        os.system("/home/alderrden/gcc-x86/bin/x86_64-elf-gcc -I./include  -m32 -std=gnu99 -ffreestanding -Wall -Wextra -c kernel.c -o ./obj/kernel.o")
        self.link()

    def link(self):
        cprint("-= Linking =-", "blue")
        files = next(os.walk('./obj'))[2]
        obj = ' ./obj/'.join(map(str, files))
        command_link = "/home/alderrden/gcc-x86/bin/x86_64-elf-ld -m elf_i386 -T ./conf/linker.ld -nostdlib --allow-multiple-definition -o ./iso/MarsOS.bin ./obj/entry.o " + "./obj/" + obj.replace("./obj/entry.o", " ")
        print(command_link)
        os.system(command_link)
        os.system("grub-file --is-x86-multiboot ./iso/MarsOS.bin")
        time.sleep(0.2)
        check = next(os.walk('./iso'))[2]
        if check == "[]":
            cprint("-=FAILED=-", "red")
            print("Check errors in terminel")
            exit()
        else:
            self.build_iso()
 
    def build_iso(self):
        cprint("-= Building ISO =-", "blue")
        os.system("mkdir -p ./iso/boot/grub")
        os.system("cp -f ./iso/MarsOS.bin ./iso/boot/")
        os.system("cp -f ./conf/grub.cfg ./iso/boot/grub/")
        os.system("/usr/bin/grub-mkrescue -o ./iso/MarsOS.iso ./iso")
        os.system("rm -f ./iso/MarsOS.bin")
        cprint("-= Create Filesystem=-", "blue")
        cprint("Please write your pass for start mkdisk.sh", "yellow")
        os.system("sudo ./mkdisk.sh")
        cprint("-= FINISH =-", "green")
        qemu = input("You want start os in qemu? [y/n] > ")
        if qemu == "y":
            os.system("sudo qemu-system-i386 -cdrom ./iso/MarsOS.iso -drive file=ext2.img,format=raw")
        if qemu == "n":
            cprint("Good bye!", "green")
            exit()

if __name__ == '__main__':
    compil()