#pragma once

int raw_isrxempty(int channel);
int raw_getc(int c);
int raw_istxready(int c);
void raw_putc(int chan, char c);
int raw_putcc(int chan, char c);
void raw_logemergency(int chan, char* c);

void bwputc(int channel, char c);
char c2x(char ch);
void bwputx(int channel, char c);
void bwputr(int channel, unsigned int reg);
void bwputstr(int channel, char *str);
void bwputw(int channel, int n, char fc, char *bf);
int bwgetc(int channel);
int bwa2d(char ch);
char bwa2i(char ch, char **src, int base, int *nump);
void bwui2a(unsigned int num, unsigned int base, char *bf);
void bwi2a(int num, char *bf);
void bwformat(int channel, char *fmt, va_list va);
void bwprintf(int channel, char *fmt, ... );
