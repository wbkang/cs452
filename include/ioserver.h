#pragma once

#define IOSERVER_ERROR_BADREQNO -4
#define IOSERVER_ERROR_BADDATA -5

int ioserver_create(int channel, int fifo, int speed, int stopbits, int databits, int parity);

int ioserver_getc(int tid);
int ioserver_putc(char c, int tid);
int ioserver_putstr(char const * c, int tid);
