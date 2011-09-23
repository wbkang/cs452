#pragma once

int raw_isrxempty(int channel);
int raw_getc(int c);
int raw_istxready(int c);
void raw_putc(int chan, char c);
void raw_init();

void bwputc(int channel, char c);
void bwputx(int channel, char c);
void bwputr(int channel, unsigned int reg);
void bwputstr(int channel, char *str);
void bwputw(int channel, int n, char fc, char *bf);
void bwformat(int channel, char *fmt, va_list va);
void bwprintf(int channel, char *fmt, ... );

