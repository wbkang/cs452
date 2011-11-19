#pragma once

#include <util.h>

int strcmp(const char *s1, const char *s2);
char char2hex(char n);
int char2digit(char ch);
void uint2str(uint num, uint base, char *bf);
void int2str(int num, char *bf);
uint strlen(char const *str);
char* strcpy(char *dest, const char *src);
char* strncpy(char *dest, const char *src, uint count);
int memcmp(const void* p1, const void* p2, uint count);
uint strparseuint(char *str, int *idx);
uint strgetui(char **c);
uint strgetw(char src[], char dst[], int lim);
void* memcpy(void* destination, const void* source, uint num);
