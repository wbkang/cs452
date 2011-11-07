#pragma once
#include <console.h>

#define LEN_CMD 32

typedef void (*cmdprocessor)(void * state, char* buf, int len);

typedef struct {
	int line, col;
	console *con;
	int cmdidx;
	char cmdbuf[LEN_CMD];
	cmdprocessor cmdprocessor;
	void *cmdprocstate;
} cmdline;

void cmdline_create(cmdline *cmd, int line, int col, console *con, cmdprocessor cmdproc, void* state);
void cmdline_handleinput(cmdline *cmd, char c);
void cmdline_clear(cmdline *cmd);
