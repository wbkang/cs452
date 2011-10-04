#pragma once

#include <util.h>
#include <assembly.h>

#define SENDER_MSGLEN(a4) ((a4) & 0xFFFF)
#define SENDER_REPLYLEN(a4) (((uint ) a4) >> 16)

int Send(int tid, char *msg, int msglen, char *reply, int replylen);
