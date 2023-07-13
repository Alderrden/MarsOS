#include "vesa.h"
#include "bitmap.h"
#include "font.h"
#include "string.h"
#include "types.h"
#include "tty.h"

int x = 0;
int x2 = 0;
int y = 0;

void set_printing_coords(int dx, int dy)
{
    x = dx;
    x2 = x;
    y = dy;
}

void print_ch(char c)
{
    int lx; int ly;
    uint8_t *bitmap = font8x8_basic[c % 128];
    for (lx = 0; lx < GLYPH_WIDTH; lx++) {
        for (ly = 0; ly < GLYPH_HEIGHT; ly++) {
            uint8_t row = bitmap[ly];
            if ((row >> lx) & 1)
                vbe_putpixel(x+lx, y+ly, VBE_RGB(255, 255, 255));
            else
                vbe_putpixel(x+lx, y+ly, VBE_RGB(0, 0, 255));
        }
    }
}

void put_char(char c)
{
    if (c == '\n')
    {
        y += GLYPH_HEIGHT;
        x = x2;
    } else if (c == '\b')
    {
        x -= GLYPH_WIDTH;
    } else if (c == '\r')
    {
        x = x2;
    } else
    {
        print_ch(c);
        x += GLYPH_WIDTH;
    }
}



void put_string(char *s)
{
    size_t l = strlen(s);
    for (size_t i = 0; i < l; i++)
    {
        char c = s[i];
        put_char(c);
    }
}

void vprintf(const char *format, ...) {
    char **arg = (char **)&format;
    int c;
    char buf[32];

    arg++;

    memset(buf, 0, sizeof(buf));
    while ((c = *format++) != 0) {
        if (c != '%')
            put_char(c);
        else {
            char *p, *p2;
            int pad0 = 0, pad = 0;

            c = *format++;
            if (c == '0') {
                pad0 = 1;
                c = *format++;
            }

            if (c >= '0' && c <= '9') {
                pad = c - '0';
                c = *format++;
            }

            switch (c) {
                case 'd':
                case 'u':
                case 'x':
                    itoa(buf, c, *((int *)arg++));
                    p = buf;
                    goto string;
                    break;

                case 's':
                    p = *arg++;
                    if (!p)
                        p = "(null)";

                string:
                    for (p2 = p; *p2; p2++)
                        ;
                    for (; p2 < p + pad; p2++)
                        put_char(pad0 ? '0' : ' ');
                    while (*p)
                        put_char(*p++);
                    break;

                default:
                    put_char(*((int *)arg++));
                    break;
            }
        }
    }
}

// read string from console, but no backing
void vgetstr(char *buffer) {
    if (!buffer) return;
    while(1) {
        char ch = kb_getchar();
        if (ch == '\n') {
            vprintf("\n");
            return ;
        } else {
            *buffer++ = ch;
            vprintf("%c", ch);
        }
    }
}