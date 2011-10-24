#pragma once

#include <util.h>

char char2hex(char n);
int char2digit(char ch);
void uint2str(uint num, uint base, char *bf);
void int2str(int num, char *bf);
uint strlen(char const *str);
void strcpy(char *dest, const char *src);
int memcmp(const void* p1, const void* p2, uint count);
uint strparseuint(char *str, int *idx);
uint strgetui(char **c);
void* memcpy(void* destination, const void* source, uint num);
