#pragma once



int raw_isrxempty(int channel);
int raw_getc(int c);
int raw_istxready(int c);
void raw_putc(int chan, char c);
int raw_putcc(int chan, char c);
void raw_logemergency(int chan, char* c);

