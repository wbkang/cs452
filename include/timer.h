#pragma once

struct timeout_info {
	void (*callback)(void*);
	void *arg;
	unsigned int start_time;
	unsigned int timeout;
};

typedef void (*timeout_callback)(void*);

struct timeout_info timer_poptimeout();

void timer3_init();

int timer_settimeout(void (*fn)(void*), void* arg, unsigned int timeoutms);
void timer_cleartimeout(int id);

unsigned int timer3_getvalue();

void sleep(int ms);


int timer_timeoutcount();
