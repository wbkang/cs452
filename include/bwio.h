#pragma once 
/*
 * bwio.h
 */
#include <ts7200.h>
#include <util.h>

int bwpopoutbuf(int channel);
void bwwriteoutbuf(int channel, char c);

void bwinit();

int bwsetfifo( int channel, int state );

int bwsetspeed( int channel, int speed );

int bwputc( int channel, char c );

int bwgetc( int channel );

int bwputx( int channel, char c );

int bwputstr( int channel, char *str );

int bwputr( int channel, unsigned int reg );

void bwputw( int channel, int n, char fc, char *bf );

void bwprintf( int channel, char *format, ... );
void bwformat(int channel, char *fmt, va_list vl);

int bwtryputc(int channel);
