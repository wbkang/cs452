#include <syscall.h>
#include <assembly.h>
#include <nameserver.h>
#include <timeserver.h>

#define MAX_MSG_LEN 0xffff

inline int Send(int tid, char *msg, int msglen, char *reply, int replylen) {
	if ((msglen | replylen) & 0xffff0000) return -3;
	return asm_Send(tid, msg, reply, (replylen << 16) | msglen);
}

int RegisterAs(char *name) {
	return nameserver_registeras(name);
}

int WhoIs(char *name) {
	return nameserver_whois(name);
}

int Time() {
	return timeserver_time();
}

int Delay(int ticks) {
	return timeserver_delay(ticks);
}

int DelayUntil(int ticks) {
	return timeserver_delayuntil(ticks);
}
