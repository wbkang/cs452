#pragma once

//////////// MACHINE STUFF
#define COM1    0
#define COM2    1
#define COM_COUNT 2

#define UART_BASE(_x) (((_x) == COM1) ? UART1_BASE : UART2_BASE)

#define ON  1
#define OFF 0

extern unsigned int _MY_MEM_START;

//////////// METHODS

void uart_fifo(int channel, int state);
void uart_stopbits(int channel, int bits);
void uart_databits(int channel, int wlen);
void uart_parity(int channel, int enable);
void uart_speed(int channel, int speed);
