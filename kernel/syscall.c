#include <syscall.h>

#define MAX_MSG_LEN 0xFFFF

int Send(int tid, char *msg, int msglen, char *reply, int replylen) {
	ASSERT(msglen > 0 && msglen <= MAX_MSG_LEN, "message length out of bounds");
	ASSERT(replylen > 0 && replylen <= MAX_MSG_LEN, "reply length out of bounds");
	uint lengths = (((uint) replylen) << 16) | msglen;
	return asm_Send(tid, msg, reply, lengths);
}
