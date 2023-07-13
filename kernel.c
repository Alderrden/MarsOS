#include "kernel.h"

#define PRINT_MODES 1
#define BRAND_QEMU  1
#define BRAND_VBOX  2
char buffer[256];
const char *shell = "mars@os>";
KERNEL_MEMORY_MAP g_kmap;

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
        vprintf("Brand: %s\n", brand);
        for(type = 0; type < 4; type++) {
            __cpuid(type, &eax, &ebx, &ecx, &edx);
            vprintf("type:0x%x, eax:0x%x, ebx:0x%x, ecx:0x%x, edx:0x%x\n", type, eax, ebx, ecx, edx);
        }
    }

    if (strstr(brand, "QEMU") != NULL)
        return BRAND_QEMU;

    return BRAND_VBOX;
}

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
    vprintf("kernel:\n");
    vprintf("  kernel-start: 0x%x, kernel-end: 0x%x, TOTAL: %d bytes\n",
           kmap->kernel.k_start_addr, kmap->kernel.k_end_addr, kmap->kernel.k_len);
    vprintf("  text-start: 0x%x, text-end: 0x%x, TOTAL: %d bytes\n",
           kmap->kernel.text_start_addr, kmap->kernel.text_end_addr, kmap->kernel.text_len);
    vprintf("  data-start: 0x%x, data-end: 0x%x, TOTAL: %d bytes\n",
           kmap->kernel.data_start_addr, kmap->kernel.data_end_addr, kmap->kernel.data_len);
    vprintf("  rodata-start: 0x%x, rodata-end: 0x%x, TOTAL: %d\n",
           kmap->kernel.rodata_start_addr, kmap->kernel.rodata_end_addr, kmap->kernel.rodata_len);
    vprintf("  bss-start: 0x%x, bss-end: 0x%x, TOTAL: %d\n",
           kmap->kernel.bss_start_addr, kmap->kernel.bss_end_addr, kmap->kernel.bss_len);

    vprintf("total_memory: %d KB\n", kmap->system.total_memory);
    vprintf("available:\n");
    vprintf("  start_adddr: 0x%x\n  end_addr: 0x%x\n  size: %d\n",
           kmap->available.start_addr, kmap->available.end_addr, kmap->available.size);
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

void ide_test() {
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
    strcpy(e.name, "MarsOS");

    // write message to drive
    strcpy(buf, "Hello World! I CAN!");
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

void mouse_test () {
    vga_disable_cursor();
    keyboard_init();
    mouse_init();

    printf("\n\n\t\t--Mouse Click Here--");
}

void vesa_tty() {
    vesa_init(1024, 768, 32);
    gimp_bitmap(desktop, desktop_h, desktop_w);
    //gimp_bitmap(cur_data, cur_h, cur_w);
    set_printing_coords(1, 1);
    vprintf("MarsOS v.0.0.1 (beta)\n");
    vprintf("by Alderrden :)\n");
    //DateTime now = rtc_get_datetime();
    //vprintf("Time:\t\t", "%2d/%2d/%2d - %2d:%2d:%2d", now.date.d,
    //            now.date.m, now.date.y, now.time.h, now.time.m, now.time.s);
    vprintf('\n');
    while(1) {
        vprintf(shell);
        memset(buffer, 0, sizeof(buffer));
        vgetstr(buffer);
        //getStr(buffer, 256);
        if (strlen(buffer) == 0)
            continue;
        if(strcmp(buffer, "cpuid") == 0) {
            cpuid_info(1);
        } else if(strcmp(buffer, "help") == 0) {
            vprintf("Tiny OS Terminal\n");
            vprintf("Commands: help, cpuid, echo, shutdown\n");
        } else if(is_echo(buffer)) {
            vprintf("%s\n", buffer + 5);
        } else if(strcmp(buffer, "shutdown") == 0) {
            shutdown();
        } else if(strcmp(buffer, "ide") == 0) {
            ide_test();
        } else if(strcmp(buffer, "mouse") == 0) {
            mouse_test();
        //} else if(strcmp(buffer, "sound") == 0) {
        //    play_thunderstruck();
        } else if(strcmp(buffer, "clean vbe") == 0) {
            vesa_clear(VBE_RGB(0, 0, 0));
        //} else if(strcmp(buffer, "pong") == 0) {
        //S    pong_game();
        //}else if(strcmp(buffer, "cat") == 0) {
            //printFileCMD();
        //}else if(strcmp(buffer, "ls") == 0) {
            //listFiles();
        //}else if(strcmp(buffer, "cd") == 0) {
            //changeDirCMD();
        } else if(strcmp(buffer, "vesa modes") == 0) {
            vesa_modes();
        } else {
            vprintf("invalid command: %s\n", buffer);
        }
    }

}

void vesa_modes() {
    vesa_init(1024, 768, 32);
}

void vga_console(){
    //printf("MarsOS Old debug shell. For strar new termilal write:\n");
    //printf("vesa tty\n");
    while(1) {
        printf(shell);
        memset(buffer, 0, sizeof(buffer));
        getstr_bound(buffer, strlen(shell));
        //getStr(buffer, 256);
        if (strlen(buffer) == 0)
            continue;
        if(strcmp(buffer, "cpuid") == 0) {
            cpuid_info(1);
        } else if(strcmp(buffer, "help") == 0) {
            printf("Tiny OS Terminal\n");
            printf("Commands: help, cpuid, echo, shutdown\n");
        } else if(is_echo(buffer)) {
            printf("%s\n", buffer + 5);
        } else if(strcmp(buffer, "shutdown") == 0) {
            shutdown();
        } else if(strcmp(buffer, "ide") == 0) {
            ide_test();
        } else if(strcmp(buffer, "mouse") == 0) {
            mouse_test();
        //}else if(strcmp(buffer, "cat") == 0) {
            //printFileCMD();
        //}else if(strcmp(buffer, "ls") == 0) {
            //listFiles();
        //}else if(strcmp(buffer, "cd") == 0) {
            //changeDirCMD();
        } else if(strcmp(buffer, "vesa tty") == 0) {
            vesa_tty();
        } else if(strcmp(buffer, "vesa modes") == 0) {
            vesa_modes();
        } else {
            printf("invalid command: %s\n", buffer);
        }
    }
}

void kmain(unsigned long magic, unsigned long addr) {
    MULTIBOOT_INFO *mboot_info;
    gdt_init();
    idt_init();
    //loadFileSystem();

    console_init(COLOR_WHITE, COLOR_BLACK);
    keyboard_init();
    //printf("Welcome to MarsOS\n");
    //printf("Kernel start\n");
    //printf("Get kernel memory map...\n");
    if (magic == MULTIBOOT_BOOTLOADER_MAGIC) {
        mboot_info = (MULTIBOOT_INFO *)addr;
        memset(&g_kmap, 0, sizeof(KERNEL_MEMORY_MAP));
        if (get_kernel_memory_map(&g_kmap, mboot_info) < 0) {
            printf("[ERROR] failed to get kernel memory map\n");
            return;
        }
        //printf("Put the memory bitmap at the start of the available memory\n");
        pmm_init(g_kmap.available.start_addr, g_kmap.available.size);
        // initialize atleast 1MB blocks of memory for our heap
        pmm_init_region(g_kmap.available.start_addr, PMM_BLOCK_SIZE * 256);
        // initialize heap 256 blocks(1MB)
        void *start = pmm_alloc_blocks(256);
        void *end = start + (pmm_next_free_frame(1) * PMM_BLOCK_SIZE);
        kheap_init(start, end);
        pmm_free_blocks(start, 256);
        pmm_deinit_region(g_kmap.available.start_addr, PMM_BLOCK_SIZE * 256);
        vga_console();
    } else {
        printf("[ERROR] invalid multiboot magic number\n");
    }
}