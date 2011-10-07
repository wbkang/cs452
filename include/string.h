#pragma once

#include <util.h>

char char2hex(char n);

int char2digit(char ch);

void uint2str(uint num, uint base, char *bf);

void int2str(int num, char *bf);

uint strlen(char *str);

void strcpy(char *dest, const char *src);

uint strparseuint(char *str, int *idx);

void* memcpy(void* destination, const void* source, uint num);

int memcmp(const void *p1, const void *p2, uint count);
