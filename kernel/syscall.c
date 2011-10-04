#include <syscall.h>
#include <string.h>
#include <nameserver.h>

#define MAX_MSG_LEN 0xFFFF

int Send(int tid, char *msg, int msglen, char *reply, int replylen) {
	ASSERT(msglen > 0 && msglen <= MAX_MSG_LEN, "message length out of bounds");
	ASSERT(replylen > 0 && replylen <= MAX_MSG_LEN, "reply length out of bounds");
	uint lengths = (((uint) replylen) << 16) | msglen;
	return asm_Send(tid, msg, reply, lengths);
}

int RegisterAs(char *name) { // wrapper for send
	nameserver_request req;
	req.n = NAMESERVER_REQUEST_REGISTERAS;
	ASSERT(NAMESERVER_MAX_NAME_LEN >= strlen(name) + 1, "name too long");
	strcpy(req.str, name);
	int rv;
	Send(NameServerTid(), (void*) &req, sizeof req, (void*) &rv, sizeof rv);
	return rv;
}

int WhoIs(char *name) { // wrapper for send
	nameserver_request req;
	req.n = NAMESERVER_REQUEST_WHOIS;
	ASSERT(NAMESERVER_MAX_NAME_LEN >= strlen(name) + 1, "name too long");
	strcpy(req.str, name);
	int rv;
	Send(NameServerTid(), (void*) &req, sizeof req, (void*) &rv, sizeof rv);
	return rv;
}
