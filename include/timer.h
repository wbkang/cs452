#ifndef TIMER_H_
#define TIMER_H_

void timer3_init();

void timer_settimeout(void (*fn)(void*), int timeoutms);

unsigned int timer3_getvalue();

void sleep(int ms);
#endif
