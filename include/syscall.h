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

int Send(int tid, char *msg, int msglen, char *reply, int replylen);
int RegisterAs(char *name);
int WhoIs(char *name);
int Time(int timeserver);
int Delay(int ticks, int tid);
int DelayUntil(int ticks, int tid);
int Getc(int channel, int tid);
int Putc(int channel, char c, int tid);
int Putstr(int channel, char const *str, int tid);
int ReplyInt(int tid, int rv);
int ReplyNull(int tid);
int SendNull(int tid);

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
