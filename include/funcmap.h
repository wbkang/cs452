#pragma once

typedef struct _tag_funcinfo {
	unsigned int fn;
	char *name;
} funcinfo;

void __init_funclist();
funcinfo *__getfunclist();
