#include <syscall.h>
#include <assembly.h>
#include <nameserver.h>
#include <timeserver.h>
#include <ioserver.h>

int Send(int tid, char *msg, int msglen, char *reply, int replylen) {
	if ((msglen | replylen) & 0xffff0000) return -3;
	return asm_Send(tid, msg, reply, (replylen << 16) | msglen);
}

int RegisterAs(char *name) {
	return nameserver_registeras(name);
}

int WhoIs(char *name) {
	return nameserver_whois(name);
}

int Time(int timeserver) {
	return timeserver_time(timeserver);
}

int Delay(int ticks, int tid) {
	return timeserver_delay(ticks, tid);
}

int DelayUntil(int ticks, int tid) {
	return timeserver_delayuntil(ticks, tid);
}

int Getc(int channel, int tid) {
	(void) channel;
	return ioserver_getc(tid);
}

int Putc(int channel, char c, int tid) {
	(void) channel;
	return ioserver_putc(c, tid);
}

int ReplyInt(int tid, int rv) {
	return Reply(tid, &rv, sizeof rv);
}

int ReplyNull(int tid) {
	return Reply(tid, NULL, 0);
}

int SendNull(int tid) {
	return Send(tid, NULL, 0, NULL, 0);
}
