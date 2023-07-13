#include "vesa.h"
#include "bios32.h"
#include "console.h"
#include "io_ports.h"
#include "isr.h"
#include "string.h"
#include "types.h"

// vbe information
VBE20_INFOBLOCK g_vbe_infoblock;
VBE20_MODEINFOBLOCK g_vbe_modeinfoblock;
// selected mode
int g_selected_mode = -1;
// selected mode width & height
uint32 g_width = 0, g_height = 0;
// buffer pointer pointing to video memory
uint32 *g_vbe_buffer = NULL;

uint32 vbe_get_width() {
    return g_width;
}

uint32 vbe_get_height() {
    return g_height;
}

// get vbe info
int get_vbe_info() {
    REGISTERS16 in = {0}, out = {0};
    // set specific value 0x4F00 in ax to get vbe info into bios memory area
    in.ax = 0x4F00;
    // set address pointer to BIOS_CONVENTIONAL_MEMORY where vbe info struct will be stored
    in.di = BIOS_CONVENTIONAL_MEMORY;
    int86(0x10, &in, &out);  // call video interrupt 0x10
    // copy vbe info data to our global variable g_vbe_infoblock
    memcpy(&g_vbe_infoblock, (void *)BIOS_CONVENTIONAL_MEMORY, sizeof(VBE20_INFOBLOCK));
    return (out.ax == 0x4F);
}

void get_vbe_mode_info(uint16 mode, VBE20_MODEINFOBLOCK *mode_info) {
    REGISTERS16 in = {0}, out = {0};
    // set specific value 0x4F00 in ax to get vbe mode info into some other bios memory area
    in.ax = 0x4F01;
    in.cx = mode; // set mode info to get
    in.di = BIOS_CONVENTIONAL_MEMORY + 1024;  // address pointer, different than used in get_vbe_info()
    int86(0x10, &in, &out);  // call video interrupt 0x10
    // copy vbe mode info data to parameter mode_info
    memcpy(mode_info, (void *)BIOS_CONVENTIONAL_MEMORY + 1024, sizeof(VBE20_MODEINFOBLOCK));
}

void vbe_set_mode(uint32 mode) {
    REGISTERS16 in = {0}, out = {0};
    // set any given mode, mode is to find by resolution X & Y
    in.ax = 0x4F02;
    in.bx = mode;
    int86(0x10, &in, &out);  // call video interrupt 0x10 to set mode
}

// find the vbe mode by width & height & bits per pixel
uint32 vbe_find_mode(uint32 width, uint32 height, uint32 bpp) {
    // iterate through video modes list
    uint16 *mode_list = (uint16 *)g_vbe_infoblock.VideoModePtr;
    uint16 mode = *mode_list++;
    while (mode != 0xffff) {
        // get each mode info
        get_vbe_mode_info(mode, &g_vbe_modeinfoblock);
        if (g_vbe_modeinfoblock.XResolution == width && g_vbe_modeinfoblock.YResolution == height && g_vbe_modeinfoblock.BitsPerPixel == bpp) {
            return mode;
        }
        mode = *mode_list++;
    }
    return -1;
}

// print availabel modes to console
void vbe_print_available_modes() {
    VBE20_MODEINFOBLOCK modeinfoblock;

    // iterate through video modes list
    uint16 *mode_list = (uint16 *)g_vbe_infoblock.VideoModePtr;
    uint16 mode = *mode_list++;
    while (mode != 0xffff) {
        get_vbe_mode_info(mode, &modeinfoblock);
        printf("Mode: %d, X: %d, Y: %d\n", mode, modeinfoblock.XResolution, modeinfoblock.YResolution);
        mode = *mode_list++;
    }
}

// set rgb values in 32 bit number
uint32 vbe_rgb(uint8 red, uint8 green, uint8 blue) {
    uint32 color = red;
    color <<= 16;
    color |= (green << 8);
    color |= blue;
    return color;
}

// put the pixel on the given x,y point
void vbe_putpixel(int x, int y, int color) {
    uint32 i = y * g_width + x;
    *(g_vbe_buffer + i) = color;
}

void draw_line(int x1, int y1, int x2, int y2, int color)
{
  if(y1 == y2){
    for(int i = x1; i <= x2; i++)
      vbe_putpixel(i, y1, color);
    return;
  }

  if(x1 == x2){
    for(int i = y1; i <= y2; i++){
      vbe_putpixel(x1, i, color);
    }
    return;
  }
}

void draw_rect(int x, int y, int width, int height, int color)
{
  draw_line(x, y, x, y + height, color);
  draw_line(x, y, x + width, y, color);
  draw_line(x + width, y, x + width, y + height, color);
  draw_line(x, y + height, x + width, y + height, color);
}

void fill_rect(int x, int y, int width, int height, int color)
{
  draw_line(x, y, x, y + height, color);
  draw_line(x, y, x + width, y, color);
  draw_line(x + width, y, x + width, y + height, color);
  draw_line(x, y + height, x + width, y + height, color);
  for(int i = y; i < y + height; i++){
    draw_line(x, i, x + width, i, color);
  }
}

void draw_bresenham_circle(int xc, int yc, int x, int y, int color) 
{ 
  vbe_putpixel(xc+x, yc+y, color); 
  vbe_putpixel(xc-x, yc+y, color); 
  vbe_putpixel(xc+x, yc-y, color); 
  vbe_putpixel(xc-x, yc-y, color); 
  vbe_putpixel(xc+y, yc+x, color); 
  vbe_putpixel(xc-y, yc+x, color); 
  vbe_putpixel(xc+y, yc-x, color); 
  vbe_putpixel(xc-y, yc-x, color); 
} 

void draw_circle(int x, int y, int radius, int color)
{
  int x2 = 0, y2 = radius;
  int d = 3 - 2 * radius;
  draw_bresenham_circle(x, y, x2, y2, color);
  while(y2 >= x2){
    x2++;
    if(d > 0){
      y2--;
      d = d + 4 * (x2 - y2) + 10;
    }else{
      d = d + 4 * x2 + 6;
    } 
    draw_bresenham_circle(x, y, x2, y2, color);
  } 
}

void draw_diamond(int x, int y, int radius, int color)
{
  int x2 = 0, y2 = radius;
  int d = 3 - 2 * radius;
  draw_bresenham_circle(x, y, x2, y2, color);
  while(y2 >= x2){
    x2++;
    if(d > 0){
      y2--;
      d = d + 4 * (x2 - y2) + 10;
    }else{
      d = d + 4 * x2 + 6;
    } 
    draw_bresenham_circle(x, y, x2, y2, color);
  } 
}

void gimp_bitmap(char* logo_ptr, int h, int w)
{
    char rgb[3] = { 0 };
    /* Copy ptr because HEADER_PIXEL increases it */
    //char* logo_ptr = desktop;

    for (unsigned int y = 0; y < h; y++) {
        for (unsigned int x = 0; x < w; x++) {
            HEADER_PIXEL(logo_ptr, rgb);
            vbe_putpixel(x, y, VBE_RGB(rgb[0], rgb[1], rgb[2]));
        }
    }
}
void gimp_bitmap_xy(char* logo_ptr, int h, int w)
{
    char rgb[3] = { 0 };
    /* Copy ptr because HEADER_PIXEL increases it */
    //char* logo_ptr = desktop;

    for (unsigned int y = 0; y < h; y++) {
        for (unsigned int x = 0; x < w; x++) {
            HEADER_PIXEL(logo_ptr, rgb);
            vbe_putpixel(x, y, VBE_RGB(rgb[0], rgb[1], rgb[2]));
        }
    }
}
int vesa_init(uint32 width, uint32 height, uint32 bpp) {
    bios32_init();
    printf("initializing vesa vbe 2.0\n");
    if (!get_vbe_info()) {
        printf("No VESA VBE 2.0 detected\n");
        return -1;
    }
    // set this to 1 to print all available modes to console
    #define PRINT_MODES 0
    #if PRINT_MODES
        printf("Press UP and DOWN arrow keys to scroll\n");
        printf("Modes:\n");
        vbe_print_available_modes();
        return 1;
    #else
        g_selected_mode = vbe_find_mode(width, height, bpp);
        if (g_selected_mode == -1) {
            printf("failed to find mode for %d-%d\n", width, height);
            return -1;
        }
        printf("\nselected mode: %d \n", g_selected_mode);
        // set selection resolution to width & height
        g_width = g_vbe_modeinfoblock.XResolution;
        g_height = g_vbe_modeinfoblock.YResolution;
        // set selected mode video physical address point to buffer for pixel plotting
        g_vbe_buffer = (uint32 *)g_vbe_modeinfoblock.PhysBasePtr;
        // set the mode to start graphics window
        vbe_set_mode(g_selected_mode);
    #endif
    
    return 0;
}

void vesa_clear(int color) {
    int h = vbe_get_height();
    int w = vbe_get_width();
    for (unsigned int y = 0; y < h; y++) {
        for (unsigned int x = 0; x < w; x++) {
            vbe_putpixel(x, y, color);
        }
    }
}