#include <console.h>
#include <syscall.h>

void console_effect(console *c, int effect){
	CONSOLE_PRINTF(c,"\033[%dm", effect);
}

void console_effect_reset(console *c) {
	console_effect(c, EFFECT_RESET);
}

void console_clear(console *console) {
	CONSOLE_PRINTF(console, CONSOLE_CLEAR);
}

void console_erase_eol(console *c) {
	CONSOLE_PRINTF(c, CONSOLE_ERASE_EOL);
}

void console_move(console *c, int line, int column) {
	CONSOLE_PRINTF(c, "\x1B[%d;%dH", line, column);
}

void console_cursor_left(console *c, int count) {
	CONSOLE_PRINTF(c, "\x1B[%dD", count);
}

void console_cursor_save(console *c) {
	CONSOLE_PRINTF(c, CONSOLE_SAVECURSOR);
}

void console_cursor_unsave(console *c) {
	CONSOLE_PRINTF(c, CONSOLE_UNSAVECURSOR);
}

void console_scroll(console *c, uint start, uint end) {
	CONSOLE_PRINTF(c, "\x1B[%d;%dr", start, end);
}

void console_create(console *c, int tid) {
	c->tid_console = tid;
	c->buf_location = 0;
	console_cursor_save(c);
}

void console_flush(console *c) {
	console_cursor_unsave(c);
	c->buf[c->buf_location] = '\0';
	Putstr(COM2, c->buf, c->tid_console);
	c->buf_location = 0;
	console_cursor_save(c);
}

//void console_fillrect(console *buf, char c, int x, int y, int width, int height) {
//	char *orig_buf = buf;
//	for (int i = y; i < y + height; i++) {
//		buf += console_cursor_move(buf, i, x);
//		for (int j = x; j < x + width; j++) {
//			*buf++ = c;
//		}
//	}
//	return buf - orig_buf;
//}

