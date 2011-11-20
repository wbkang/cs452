#pragma once

#include <util.h>
#include <string.h>

typedef struct _tag_funcinfo {
	unsigned int fn;
	char *name;
} funcinfo;

void __init_funclist();
funcinfo *__getfunclist();


static inline char* find_function_name(uint pc) {
	funcinfo* fl = __getfunclist();
	int i = 0;

	while (fl[i].fn != 0) {
		if (fl[i].fn == pc) return fl[i].name;
		i++;
	}

	return "[unknown function]";
}

static inline func_t find_func_addr(char *name) {
	funcinfo* fl = __getfunclist();
	int i = 0;

	while (fl[i].fn != 0) {
		if (strcmp(fl[i].name, name) == 0) return (func_t)fl[i].fn;
		i++;
	}

	return NULL;
}
