#include <server/traincmdrunner.h>
#include <syscall.h>
#include <train.h>
#include <uconst.h>
#include <server/traincmdbuffer.h>
#include <server/publisher.h>
#include <server/switchcmdrunner.h>
#include <server/buffertask.h>
#include <server/courier.h>

void traincmdrunner() {
	int tid_com1 = WhoIs(NAME_IOSERVER_COM1);
	int tid_time = WhoIs(NAME_TIMESERVER);
	int tid_traincmdbuf = WhoIs(NAME_TRAINCMDBUFFER);
	courier_new(PRIORITY_TRAINCMDRUNNER, tid_traincmdbuf, MyTid(), sizeof(traincmd));
	int tid_traincmdpub = WhoIs(NAME_TRAINCMDPUB);
	int tid_switchcmdrunner = switchcmdrunner_new(NAME_SWITCHCMDRUNNER);

	traincmd_receipt rcpt;
	rcpt.type = MSG_TRAINCMDRECEIPT;
	traincmd *cmd = &rcpt.cmd;

	for (;;) {
		int tid;
		int len = Receive(&tid, cmd, sizeof(traincmd));
		Reply(tid, NULL, 0);
		ASSERT(len == sizeof(traincmd), "bad data");
		switch (cmd->name) {
			case SPEED: {
				char train = cmd->arg1;
				// @TODO: consider 76 to honk, 64 to stop honking
				char speed = cmd->arg2 | 16; // @TODO: don't hardcode headlights
				Putc(COM1, speed, tid_com1);
				Putc(COM1, train, tid_com1);
				break;
			}
			case REVERSE: {
				char train = cmd->arg1;
				Putc(COM1, TRAIN_REVERSE, tid_com1);
				Putc(COM1, train, tid_com1);
				Delay(TRAIN_PAUSE_AFTER_REVERSE, tid_time); // @TODO: why?
				break;
			}
			case REVERSE_UI: {
				Delay(TRAIN_PAUSE_REVERSE, tid_time);
				char train = cmd->arg1;
				Putc(COM1, TRAIN_REVERSE, tid_com1);
				Putc(COM1, train, tid_com1);
				Delay(TRAIN_PAUSE_AFTER_REVERSE, tid_time); // @TODO: why?
				break;
			}
			case SWITCH: {
				int n = buffertask_put(tid_switchcmdrunner, cmd, sizeof(traincmd));
				ASSERT(n >= 0, "error sending cmd to switchrunner (%d)", n);
				continue;
			}
			case QUERY1: {
				char module = cmd->arg1;
				Putc(COM1, TRAIN_QUERYMOD | module, tid_com1);
				break;
			}
			case QUERY: {
				char modules = cmd->arg1;
				Putc(COM1, TRAIN_QUERYMODS | modules, tid_com1);
				break;
			}
			case GO:
				Putc(COM1, TRAIN_GO, tid_com1);
				break;
			case STOP:
				Putc(COM1, TRAIN_STOP, tid_com1);
				break;
			default:
				ASSERT(0, "bad train cmd: %d", cmd->name);
				break;
		}
		Flush(tid_com1);
		rcpt.timestamp = Time(tid_time);
		publisher_pub(tid_traincmdpub, &rcpt, sizeof(rcpt));
	}
}

int traincmdrunner_new() {
	return Create(PRIORITY_TRAINCMDRUNNER, traincmdrunner);
}
