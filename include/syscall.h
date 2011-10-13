#pragma once

#include <util.h>

#define SENDER_MSGLEN(a4) ((a4) & 0xffff)
#define SENDER_REPLYLEN(a4) (((uint ) a4) >> 16)

inline int Send(int tid, char *msg, int msglen, char *reply, int replylen);

inline int RegisterAs(char *name);

inline int WhoIs(char *name);

int Create(int priority, func_t code);

int MyTid();

int MyParentsTid();

void Pass();

void Exit();

void* malloc(uint size);

int Receive(int *tid, void* msg, int msglen);

int Reply(int tid, void* reply, int replylen);

int NameServerTid();

int Time();

int Delay(int ticks);

int DelayUntil(int ticks);

int AwaitEvent(int eventid);

void ExitKernel(int errno);
