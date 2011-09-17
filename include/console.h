#pragma once

#define EFFECT_RESET 0
#define EFFECT_REVERSE 7
#define EFFECT_BG_BLUE 34
#define EFFECT_FG_BLACK 30
#define EFFECT_FG_RED 31
#define EFFECT_BRIGHT 1
#define EFFECT_UNDERSCORE 4


void console_init();
void console_putstr(char * str);

void console_clear();

void console_move(int line,int column);

void console_printf(char *fmt, ...);

void console_fillrect(char c, int x, int y, int width, int height);
void console_effect(int effect);

