#pragma once

#include <string.h>

char char2hex(char n);

int char2digit(char ch);

void uint2str(unsigned int num, unsigned int base, char *bf);

void int2str(int num, char *bf);

unsigned int strlen(char *str);

unsigned int strparseuint(char *str, int *idx);
