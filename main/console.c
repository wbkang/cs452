#include <console.h>
#include <syscall.h>
#include <nameserver.h>

console *console_new(int channel) {
	console *this = malloc(sizeof(console));
	ASSERT(channel == COM2, "channel %d is not supported", channel);
	this->tid_console = WhoIs(NAME_IOSERVER_COM2);
	ASSERT(this->tid_console >= 0, "com2 ioserver not found, retval: %d", this->tid_console);
	this->buf_location = 0;
	console_cursor_save(this);
	return this;
}

void console_hidecursor(console *this) {
	console_printf(this, CONSOLE_HIDECURSOR);
}

void console_effect(console *this, int effect){
	console_printf(this,"\033[%dm", effect);
}

void console_effect_reset(console *this) {
	console_effect(this, EFFECT_RESET);
}

void console_clear(console *this) {
	console_printf(this, CONSOLE_CLEAR);
}

void console_erase_bol(console *this) {
	console_printf(this, CONSOLE_ERASE_SOL);
}

void console_erase_eol(console *this) {
	console_printf(this, CONSOLE_ERASE_EOL);
}

void console_move(console *this, int line, int column) {
	console_printf(this, "\x1B[%d;%dH", line, column);
}

void console_cursor_left(console *this, int count) {
	console_printf(this, "\x1B[%dD", count);
}

void console_cursor_save(console *this) {
	console_printf(this, CONSOLE_SAVECURSOR);
}

void console_cursor_unsave(console *this) {
	console_printf(this, CONSOLE_UNSAVECURSOR);
}

void console_scroll(console *this, uint start, uint end) {
	console_printf(this, "\x1B[%d;%dr", start, end);
}

void console_flush(console *this) {
	console_cursor_unsave(this);
	this->buf[this->buf_location] = '\0';
	Putstr(COM2, CONSOLE_HIDECURSOR, this->tid_console);
	Putstr(COM2, this->buf, this->tid_console);
	Putstr(COM2, CONSOLE_SHOWCURSOR, this->tid_console);
	this->buf_location = 0;
	console_cursor_save(this);
}

void console_flushcom(console *this) {
	Flush(this->tid_console);
}

//void console_fillrect(console *buf, char this, int x, int y, int width, int height) {
//	char *orig_buf = buf;
//	for (int i = y; i < y + height; i++) {
//		buf += console_cursor_move(buf, i, x);
//		for (int j = x; j < x + width; j++) {
//			*buf++ = this;
//		}
//	}
//	return buf - orig_buf;
//}

