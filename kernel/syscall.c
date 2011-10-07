#include <syscall.h>
#include <assembly.h>
#include <nameserver.h>

#define MAX_MSG_LEN 0xFFFF

inline int Send(int tid, char *msg, int msglen, char *reply, int replylen) {
	ASSERT(msglen > 0 && msglen <= MAX_MSG_LEN, "message length out of bounds");
	ASSERT(replylen > 0 && replylen <= MAX_MSG_LEN, "reply length out of bounds");
	return asm_Send(tid, msg, reply, (((uint) replylen) << 16) | msglen);
}

inline int RegisterAs(char *name) {
	return nameserver_send(NAMESERVER_REQUEST_REGISTERAS, name);
}

inline int WhoIs(char *name) {
	return nameserver_send(NAMESERVER_REQUEST_WHOIS, name);
}
