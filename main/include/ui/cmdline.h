#pragma once
#include <console.h>
#include <server/uiserver.h>

#define LEN_CMD 32

typedef void (*cmdprocessor)(void *this, char *buf, int len);

typedef struct {
	ui_id id_ui;
	int line, col;
	int cmdidx;
	char cmdbuf[LEN_CMD + 1];
	cmdprocessor cmdprocessor;
} cmdline;

cmdline *cmdline_new(int line, int col, cmdprocessor cmdproc);
void cmdline_handleinput(cmdline *this, char c, void* that);
void cmdline_clear(cmdline *this);
