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
	if (strlen(name) != 2 || name[0] < 'a' || name[0] > 'z' || name[1] < 'a' || name[1] > 'z') return -3;
	nameserver_request req;
	req.n = NAMESERVER_REQUEST_REGISTERAS;
	memcpy(req.str, name, 2);
	int rv;
	Send(NameServerTid(), (void*) &req, sizeof req, (void*) &rv, sizeof rv);
	return rv;
}

int WhoIs(char *name) { // wrapper for send
	if (strlen(name) != 2 || name[0] < 'a' || name[0] > 'z' || name[1] < 'a' || name[1] > 'z') return -3;
	nameserver_request req;
	req.n = NAMESERVER_REQUEST_WHOIS;
	memcpy(req.str, name, 2);
	int rv;
	Send(NameServerTid(), (void*) &req, sizeof req, (void*) &rv, sizeof rv);
	return rv;
}
