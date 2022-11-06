#include "kernel.h"
#include "console.h"
#include "string.h"
#include "gdt.h"
#include "idt.h"
#include "ide.h"
#include "kheap.h"
#include "multiboot.h"
#include "pmm.h"
#include "keyboard.h"
#include "io_ports.h"
#include "vesa.h"
#include "types.h"
#include "tty.h"
//#include "logo.h"

#define BRAND_QEMU  1
#define BRAND_VBOX  2

KERNEL_MEMORY_MAP g_kmap;

int get_kernel_memory_map(KERNEL_MEMORY_MAP *kmap, MULTIBOOT_INFO *mboot_info) {
    uint32 i;

    if (kmap == NULL) return -1;
    kmap->kernel.k_start_addr = (uint32)&__kernel_section_start;
    kmap->kernel.k_end_addr = (uint32)&__kernel_section_end;
    kmap->kernel.k_len = ((uint32)&__kernel_section_end - (uint32)&__kernel_section_start);

    kmap->kernel.text_start_addr = (uint32)&__kernel_text_section_start;
    kmap->kernel.text_end_addr = (uint32)&__kernel_text_section_end;
    kmap->kernel.text_len = ((uint32)&__kernel_text_section_end - (uint32)&__kernel_text_section_start);

    kmap->kernel.data_start_addr = (uint32)&__kernel_data_section_start;
    kmap->kernel.data_end_addr = (uint32)&__kernel_data_section_end;
    kmap->kernel.data_len = ((uint32)&__kernel_data_section_end - (uint32)&__kernel_data_section_start);

    kmap->kernel.rodata_start_addr = (uint32)&__kernel_rodata_section_start;
    kmap->kernel.rodata_end_addr = (uint32)&__kernel_rodata_section_end;
    kmap->kernel.rodata_len = ((uint32)&__kernel_rodata_section_end - (uint32)&__kernel_rodata_section_start);

    kmap->kernel.bss_start_addr = (uint32)&__kernel_bss_section_start;
    kmap->kernel.bss_end_addr = (uint32)&__kernel_bss_section_end;
    kmap->kernel.bss_len = ((uint32)&__kernel_bss_section_end - (uint32)&__kernel_bss_section_start);

    kmap->system.total_memory = mboot_info->mem_low + mboot_info->mem_high;

    for (i = 0; i < mboot_info->mmap_length; i += sizeof(MULTIBOOT_MEMORY_MAP)) {
        MULTIBOOT_MEMORY_MAP *mmap = (MULTIBOOT_MEMORY_MAP *)(mboot_info->mmap_addr + i);
        if (mmap->type != MULTIBOOT_MEMORY_AVAILABLE) continue;
        // make sure kernel is loaded at 0x100000 by bootloader(see linker.ld)
        if (mmap->addr_low == kmap->kernel.text_start_addr) {
            // set available memory starting from end of our kernel, leaving 1MB size for functions exceution
            kmap->available.start_addr = kmap->kernel.k_end_addr + 1024 * 1024;
            kmap->available.end_addr = mmap->addr_low + mmap->len_low;
            // get availabel memory in bytes
            kmap->available.size = kmap->available.end_addr - kmap->available.start_addr;
            return 0;
        }
    }

    return -1;
}

void display_kernel_memory_map(KERNEL_MEMORY_MAP *kmap) {
    printf("kernel:\n");
    printf("  kernel-start: 0x%x, kernel-end: 0x%x, TOTAL: %d bytes\n",
           kmap->kernel.k_start_addr, kmap->kernel.k_end_addr, kmap->kernel.k_len);
    printf("  text-start: 0x%x, text-end: 0x%x, TOTAL: %d bytes\n",
           kmap->kernel.text_start_addr, kmap->kernel.text_end_addr, kmap->kernel.text_len);
    printf("  data-start: 0x%x, data-end: 0x%x, TOTAL: %d bytes\n",
           kmap->kernel.data_start_addr, kmap->kernel.data_end_addr, kmap->kernel.data_len);
    printf("  rodata-start: 0x%x, rodata-end: 0x%x, TOTAL: %d\n",
           kmap->kernel.rodata_start_addr, kmap->kernel.rodata_end_addr, kmap->kernel.rodata_len);
    printf("  bss-start: 0x%x, bss-end: 0x%x, TOTAL: %d\n",
           kmap->kernel.bss_start_addr, kmap->kernel.bss_end_addr, kmap->kernel.bss_len);

    printf("total_memory: %d KB\n", kmap->system.total_memory);
    printf("available:\n");
    printf("  start_adddr: 0x%x\n  end_addr: 0x%x\n  size: %d\n",
           kmap->available.start_addr, kmap->available.end_addr, kmap->available.size);
}

void __cpuid(uint32 type, uint32 *eax, uint32 *ebx, uint32 *ecx, uint32 *edx) {
    asm volatile("cpuid"
                : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
                : "0"(type)); // put the type into eax
}

int cpuid_info(int print) {
    uint32 brand[12];
    uint32 eax, ebx, ecx, edx;
    uint32 type;

    memset(brand, 0, sizeof(brand));
    __cpuid(0x80000002, (uint32 *)brand+0x0, (uint32 *)brand+0x1, (uint32 *)brand+0x2, (uint32 *)brand+0x3);
    __cpuid(0x80000003, (uint32 *)brand+0x4, (uint32 *)brand+0x5, (uint32 *)brand+0x6, (uint32 *)brand+0x7);
    __cpuid(0x80000004, (uint32 *)brand+0x8, (uint32 *)brand+0x9, (uint32 *)brand+0xa, (uint32 *)brand+0xb);

    if (print) {
        printf("Brand: %s\n", brand);
        for(type = 0; type < 4; type++) {
            __cpuid(type, &eax, &ebx, &ecx, &edx);
            printf("type:0x%x, eax:0x%x, ebx:0x%x, ecx:0x%x, edx:0x%x\n", type, eax, ebx, ecx, edx);
        }
    }

    if (strstr(brand, "QEMU") != NULL)
        return BRAND_QEMU;

    return BRAND_VBOX;
}

BOOL is_echo(char *b) {
    if((b[0]=='e')&&(b[1]=='c')&&(b[2]=='h')&&(b[3]=='o'))
        if(b[4]==' '||b[4]=='\0')
            return TRUE;
    return FALSE;
}

void shutdown() {
    int brand = cpuid_info(0);
    // QEMU
    if (brand == BRAND_QEMU)
        outports(0x604, 0x2000);
    else
        // VirtualBox
        outports(0x4004, 0x3400);
}

void ata_test() {
    ata_init();
    printf("\nExample\n");
    const int DRIVE = ata_get_drive_by_model("VBOX HARDDISK");
    const uint32 LBA = 0;
    const uint8 NO_OF_SECTORS = 1;
    char buf[ATA_SECTOR_SIZE] = {0};

    struct example {
        int id;
        char name[32];
    };

    struct example e;
    e.id = 10012;
    strcpy(e.name, "Iron Man");

    // write message to drive
    strcpy(buf, "Hello World");
    ide_write_sectors(DRIVE, NO_OF_SECTORS, LBA, (uint32)buf);

    memset(buf, 0, sizeof(buf));
    memcpy(buf, &e, sizeof(e));
    ide_write_sectors(DRIVE, NO_OF_SECTORS, LBA + 1, (uint32)buf);
    printf("data written\n");

    // read message from drive
    memset(buf, 0, sizeof(buf));
    ide_read_sectors(DRIVE, NO_OF_SECTORS, LBA, (uint32)buf);
    printf("read data: %s\n", buf);

    memset(buf, 0, sizeof(buf));
    ide_read_sectors(DRIVE, NO_OF_SECTORS, LBA + 1, (uint32)buf);
    memcpy(&e, buf, sizeof(e));
    printf("id: %d, name: %s\n", e.id, e.name);

}

void vesa_draw_test() {
    int ret = vesa_init(1024, 768, 32);
    if (ret < 0) {
        printf("failed to init vesa graphics\n");
        //goto done;
    }
    if (ret == 1) {
        // scroll to top
        for(int i = 0; i < MAXIMUM_PAGES; i++)
            console_scroll(SCROLL_UP);

        while (1) {
            // add scrolling to view all modes
            char c = kb_get_scancode();
            if (c == SCAN_CODE_KEY_UP)
                console_scroll(SCROLL_UP);
            if (c == SCAN_CODE_KEY_DOWN)
                console_scroll(SCROLL_DOWN);
        }
    } else {
        draw_rect(0, 0, 1024 - 1, 768 - 1, VBE_RGB(255, 255, 0));
        draw_rect(2, 2, 30, 30, VBE_RGB(255, 255, 0));
        draw_rect(1024 - 33, 2, 30, 30, VBE_RGB(255, 255, 0));
        draw_rect(2, 768 - 33, 30, 30, VBE_RGB(255, 255, 0));
        draw_rect(1024 - 33, 768 - 33, 30, 30, VBE_RGB(255, 255, 0));
        draw_diamond(16, 16, 10, VBE_RGB(255, 255, 0));
        draw_diamond(1024 - 18, 16, 10, VBE_RGB(255, 255, 0));
        draw_diamond(16, 768 - 18, 10, VBE_RGB(255, 255, 0));
        draw_diamond(1024 - 18, 768 - 18, 10, VBE_RGB(255, 255, 0));      

        draw_line(1024 / 2, 0, 1024 / 2, 768, VBE_RGB(255, 255, 0));
        for(int i = 0; i < 50; i+=3){
            draw_circle(80, 100, 50-i, VBE_RGB(255, 255, 0));
        }  

        for(int i = 0; i < 50; i+=3){
            draw_diamond(240, 100, 50-i, VBE_RGB(255, 255, 0));
        }
    }
}

void test_tty() {
    int ret = vesa_init(1024, 768, 32);
    if (ret < 0) {
        printf("failed to init vesa graphics\n");
        //goto done;
    }
    if (ret == 1) {
        // scroll to top
        for(int i = 0; i < MAXIMUM_PAGES; i++)
            console_scroll(SCROLL_UP);

        while (1) {
            // add scrolling to view all modes
            char c = kb_get_scancode();
            if (c == SCAN_CODE_KEY_UP)
                console_scroll(SCROLL_UP);
            if (c == SCAN_CODE_KEY_DOWN)
                console_scroll(SCROLL_DOWN);
        }
    } else {
        draw_string("hello vesa!", 20, 200, VBE_RGB(255, 255, 0));
        bitmap_draw_string("bitmap vesa", 70, 500, VBE_RGB(255, 255, 0));
    }
}

void ht_test() {
    vbe_print_available_modes();
}

void kmain(unsigned long magic, unsigned long addr) {
    char buffer[255];
    const char *shell = "marsos>";

    gdt_init();
    idt_init();
    MULTIBOOT_INFO *mboot_info;
    uint32 i;
    console_init(COLOR_WHITE, COLOR_BLACK);
    keyboard_init();
    printf("MarsOS v0.1 (beta) debug terminal\n");
    printf("starting terminal...\n");
    while(1) {
        printf(shell);
        memset(buffer, 0, sizeof(buffer));
        getstr_bound(buffer, strlen(shell));
        if (strlen(buffer) == 0)
            continue;
        if(strcmp(buffer, "cpuid") == 0) {
            cpuid_info(1);
        } else if(strcmp(buffer, "help") == 0) {
            printf("MarsOS v0.1 (beta) debug terminal\n");
            printf("Commands: help, cpuid, echo, shutdown\n");
        } else if(is_echo(buffer)) {
            printf("%s\n", buffer + 5);
        } else if(strcmp(buffer, "shutdown") == 0) {
            shutdown();
        } else if(strcmp(buffer, "ata test") == 0) {
            ata_test();
        } else if(strcmp(buffer, "ker map") == 0) {
            printf("magic: 0x%x\n", magic);
            if(magic == MULTIBOOT_BOOTLOADER_MAGIC) {
                mboot_info = (MULTIBOOT_INFO *)addr;
                printf("  flags: 0x%x\n", mboot_info->flags);
                printf("  mem_low: 0x%x KB\n", mboot_info->mem_low);
                printf("  mem_high: 0x%x KB\n", mboot_info->mem_high);
                printf("  boot_device: 0x%x\n", mboot_info->boot_device);
                printf("  cmdline: %s\n", (char *)mboot_info->cmdline);
                printf("  modules_count: %d\n", mboot_info->modules_count);
                printf("  modules_addr: 0x%x\n", mboot_info->modules_addr);
                printf("  mmap_length: %d\n", mboot_info->mmap_length);
                printf("  mmap_addr: 0x%x\n", mboot_info->mmap_addr);
                printf("  memory map:-\n");
                for (i = 0; i < mboot_info->mmap_length; i += sizeof(MULTIBOOT_MEMORY_MAP)) {
                    MULTIBOOT_MEMORY_MAP *mmap = (MULTIBOOT_MEMORY_MAP *)(mboot_info->mmap_addr + i);
                    printf("    size: %d, addr: 0x%x%x, len: %d%d, type: %d\n", 
                            mmap->size, mmap->addr_low, mmap->addr_high, mmap->len_low, mmap->len_high, mmap->type);        

                    if (mmap->type == MULTIBOOT_MEMORY_AVAILABLE) {
                        /**** Available memory  ****/
                    }
                }
                printf("  boot_loader_name: %s\n", (char *)mboot_info->boot_loader_name);
                printf("  vbe_control_info: 0x%x\n", mboot_info->vbe_control_info);
                printf("  vbe_mode_info: 0x%x\n", mboot_info->vbe_mode_info);
                printf("  framebuffer_addr: 0x%x\n", mboot_info->framebuffer_addr);
                printf("  framebuffer_width: %d\n", mboot_info->framebuffer_width);
                printf("  framebuffer_height: %d\n", mboot_info->framebuffer_height);
                printf("  framebuffer_type: %d\n", mboot_info->framebuffer_type);
            } else {
                printf("invalid multiboot magic number\n");
            }
            console_scroll(SCROLL_UP);
        } else if(strcmp(buffer, "vesa test") == 0) {
            vesa_draw_test();
        } else if(strcmp(buffer, "vesa modes") == 0) {
            ht_test();
        } else if(strcmp(buffer, "vesa tty") == 0) {
            test_tty();
        } else {
            printf("invalid command: %s\n", buffer);
        }
    }
}

