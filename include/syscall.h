#pragma once

#include <util.h>

/*
 * System call numbers
 */

#define SYSCALL_CREATE 0
#define SYSCALL_MYTID 1
#define SYSCALL_MYPARENTTID 2
#define SYSCALL_PASS 3
#define SYSCALL_EXIT 4
#define SYSCALL_MALLOC 5
#define SYSCALL_SEND 6
#define SYSCALL_RECEIVE 7
#define SYSCALL_REPLY 8
#define SYSCALL_NAMESERVERTID 9
#define SYSCALL_AWAITEVENT 10
#define SYSCALL_EXITKERNEL 11

#define SENDER_MSGLEN(a4) ((a4) & 0xffff)
#define SENDER_REPLYLEN(a4) (((uint ) a4) >> 16)

inline int Send(int tid, char *msg, int msglen, char *reply, int replylen);
inline int RegisterAs(char *name);
inline int WhoIs(char *name);
inline int Time(int timeserver);
inline int Delay(int ticks, int timeserver);
inline int DelayUntil(int ticks, int timeserver);

int Create(int priority, func_t code);
int MyTid();
int MyParentsTid();
void Pass();
void Exit();
void* malloc(uint size);
int Receive(int *tid, void* msg, int msglen);
int Reply(int tid, void* reply, int replylen);
int NameServerTid();
int AwaitEvent(int eventid);
void ExitKernel(int errno);
