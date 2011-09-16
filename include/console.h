#ifndef CONSOLE_H_

void console_init();
void console_putstr(char * str);

void console_clear();

void console_move(int line,int column);

void console_printf(char *fmt, ...);
#endif

