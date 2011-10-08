#include <syscall.h>
#include <assembly.h>
#include <nameserver.h>
#include <timeserver.h>

#define MAX_MSG_LEN 0xffff

inline int Send(int tid, char *msg, int msglen, char *reply, int replylen) {
	ASSERT(msglen ^ 0xffff0000, "message length out of bounds");
	ASSERT(replylen ^ 0xffff0000, "reply length out of bounds");
	return asm_Send(tid, msg, reply, (((uint) replylen) << 16) | msglen);
}

inline int RegisterAs(char *name) {
	return nameserver_send(NAMESERVER_REQUEST_REGISTERAS, name);
}

inline int WhoIs(char *name) {
	return nameserver_send(NAMESERVER_REQUEST_WHOIS, name);
}

inline int Time() {
	return timeserver_time();
}

inline int Delay(int ticks) {
	return timeserver_delay(ticks);
}

inline int DelayUntil(int ticks) {
	return timeserver_delayuntil(ticks);
}
