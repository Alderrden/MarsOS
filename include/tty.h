#ifndef TTY_H
#define TTY_H

#include "vesa.h"
#include "types.h"
#include "string.h"

void set_printing_coords(int dx, int dy);

void print_ch(char c);

void put_char(char c);

void put_string(char *s);

void vprintf(const char *format, ...);

void vgetstr(char *buffer);

#endif