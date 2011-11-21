#include <syscall.h>
#include <train.h>
#include <uconst.h>
#include <server/traincmdrunner.h>
#include <server/traincmdbuffer.h>
#include <server/publisher.h>

void traincmdrunner() {
	int tid_com1 = WhoIs(NAME_IOSERVER_COM1);
	int tid_time = WhoIs(NAME_TIMESERVER);
	int tid_traincmdbuf = WhoIs(NAME_TRAINCMDBUFFER);
	int tid_traincmdpub = publisher_new(NAME_TRAINCMDPUB, PRIORITY_TRAINCMDPUB, sizeof(traincmd_receipt));

	traincmd_receipt rcpt;
	rcpt.type = TRAINCMDRECEIPT;
	traincmd *cmd = &rcpt.cmd;

	for (;;) {
		traincmdbuffer_get(tid_traincmdbuf, cmd);
		switch (cmd->name) {
			case SPEED: {
				char train = cmd->arg1;
				char speed = cmd->arg2;
				Putc(COM1, speed, tid_com1);
				Putc(COM1, train, tid_com1);
				break;
			}
			case REVERSE: {
				char train = cmd->arg1;
				Putc(COM1, TRAIN_REVERSE, tid_com1);
				Putc(COM1, train, tid_com1);
				break;
			}
			case SWITCH: {
				char swaddr = cmd->arg1;
				char swpos = cmd->arg2;
				int S = track_switchpos_straight(swpos);
				Putc(COM1, S ? TRAIN_SWITCH_STRAIGHT : TRAIN_SWITCH_CURVED, tid_com1);
				Putc(COM1, swaddr, tid_com1);
				break;
			}
			case SOLENOID:
				Putc(COM1, TRAIN_SOLENOID_OFF, tid_com1);
				break;
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
			case PAUSE: {
				int ticks = cmd->arg1;
				Delay(ticks, tid_time);
				break;
			}
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
