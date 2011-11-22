#include <syscall.h>
#include <train.h>
#include <uconst.h>
#include <server/traincmdrunner.h>
#include <server/traincmdbuffer.h>
#include <server/publisher.h>
#include <server/switchcmdrunner.h>

void traincmdrunner() {
	int tid_com1 = WhoIs(NAME_IOSERVER_COM1);
	int tid_time = WhoIs(NAME_TIMESERVER);
	int tid_traincmdbuf = WhoIs(NAME_TRAINCMDBUFFER);
	int tid_traincmdpub = publisher_new(NAME_TRAINCMDPUB, PRIORITY_TRAINCMDPUB, sizeof(traincmd_receipt));
	int tid_switchcmdrunner = switchcmdrunner_new(NULL);

	traincmd_receipt rcpt;
	rcpt.type = TRAINCMDRECEIPT;
	traincmd *cmd = &rcpt.cmd;

	for (;;) {
		traincmdbuffer_get(tid_traincmdbuf, cmd);
		switch (cmd->name) {
			case SPEED: {
				char train = cmd->arg1;
				char speed = cmd->arg2 | 16; // @TODO: don't hardcode headlights
				Putc(COM1, speed, tid_com1);
				Putc(COM1, train, tid_com1);
				break;
			}
			case REVERSE: {
				// Delay(TRAIN_PAUSE_REVERSE, tid_time);
				char train = cmd->arg1;
				Putc(COM1, TRAIN_REVERSE, tid_com1);
				Putc(COM1, train, tid_com1);
				// Delay(TRAIN_PAUSE_AFTER_REVERSE, tid_time); // @TODO: why?
				break;
			}
			case SWITCH: {
				int n = Send(tid_switchcmdrunner, cmd, sizeof(traincmd), NULL, 0);
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
				ERROR("bad train cmd: %d", cmd->name);
				break;
		}
		Flush(tid_com1);
		rcpt.timestamp = Time(tid_time);
		publisher_pub(tid_traincmdpub, &rcpt, sizeof(rcpt));
	}
}

int traincmdrunner_new(int tid_buffer) {
	return Create(PRIORITY_TRAINCMDRUNNER, traincmdrunner);
}
