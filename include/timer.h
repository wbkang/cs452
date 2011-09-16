#pragma once

struct timeout_info {
	void (*callback)(void*);
	void *arg;
	unsigned int start_time;
	int timeout;
};



void timer3_init();

void timer_settimeout(void (*fn)(void*), void* arg, int timeoutms);

unsigned int timer3_getvalue();

void sleep(int ms);



