#pragma once
#include <console.h>

#define LEN_CMD 32

typedef void (*cmdprocessor)(void* state, char *buf, int len);

typedef struct {
	console *con;
	int line, col;
	int cmdidx;
	char cmdbuf[LEN_CMD];
	cmdprocessor cmdprocessor;
	void *cmdprocstate;
} cmdline;

cmdline *cmdline_new(console *con, int line, int col, cmdprocessor cmdproc, void* state);
void cmdline_handleinput(cmdline *this, char c);
void cmdline_clear(cmdline *this);
