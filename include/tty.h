#ifndef TTY_H
#define TTY_H

#include "vesa.h"
#include "types.h"
#include "string.h"

void draw_char(char ch, int x, int y, int color);

void draw_string(const char *str, int x, int y, int color);

#endif