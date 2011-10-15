#include <syscall.h>
#include <assembly.h>
#include <nameserver.h>
#include <timeserver.h>

inline int Send(int tid, char *msg, int msglen, char *reply, int replylen) {
	if ((msglen | replylen) & 0xffff0000) return -3;
	return asm_Send(tid, msg, reply, (replylen << 16) | msglen);
}

inline int RegisterAs(char *name) {
	return nameserver_registeras(name);
}

inline int WhoIs(char *name) {
	return nameserver_whois(name);
}

inline int Time(int timeserver) {
	return timeserver_time(timeserver);
}

inline int Delay(int ticks, int timeserver) {
	return timeserver_delay(ticks, timeserver);
}

inline int DelayUntil(int ticks, int timeserver) {
	return timeserver_delayuntil(ticks, timeserver);
}
