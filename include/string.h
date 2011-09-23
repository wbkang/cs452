#pragma once

#include <util.h>

char char2hex(char n);

int char2digit(char ch);

void uint2str(uint num, uint base, char *bf);

void int2str(int num, char *bf);

uint strlen(char *str);

uint strparseuint(char *str, int *idx);
