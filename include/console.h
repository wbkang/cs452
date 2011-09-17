#pragma once

#define EFFECT_RESET 0
#define EFFECT_REVERSE 7
#define EFFECT_FG_BLUE 34
#define EFFECT_FG_BLACK 30
#define EFFECT_FG_RED 31
#define EFFECT_BRIGHT 1
#define EFFECT_UNDERSCORE 4
#define EFFECT_FG_YELLOW 33
#define EFFECT_FG_GREEN 32
#define EFFECT_FG_WHITE 37


void console_init();
void console_putstr(char * str);

void console_clear();

void console_move(int line,int column);

void console_printf(char *fmt, ...);

void console_fillrect(char c, int x, int y, int width, int height);
void console_effect(int effect);
void console_putc(char c);
void console_showcursor(int show);

void console_savecursor(int save);
void console_close();
