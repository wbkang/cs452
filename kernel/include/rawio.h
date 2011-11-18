#pragma once

#include <util.h>

int raw_isrxempty(int channel);
int raw_getc(int channel);
int raw_istxready(int channel);
void raw_putc(int channel, char c);
void raw_init();

void bwputc(int channel, char c);
int bwgetc(int channel);
void bwputx(int channel, char c);
void bwputr(int channel, uint reg);
void bwputstr(int channel, char *str);
void bwputw(int channel, int n, char fc, char *bf);
void bwformat(int channel, char *fmt, va_list va);
void bwprintf(int channel, char *fmt, ...);
