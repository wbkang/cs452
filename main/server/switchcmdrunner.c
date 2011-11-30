#include <server/switchcmdrunner.h>
#include <syscall.h>
#include <train.h>
#include <uconst.h>
#include <server/publisher.h>
#include <server/buffertask.h>
#include <server/courier.h>

void switchcmdrunner() {
	RegisterAs(NAME_SWITCHCMDRUNNER_S);
	int tid_com1 = WhoIs(NAME_IOSERVER_COM1);
	int tid_time = WhoIs(NAME_TIMESERVER);
	int tid_traincmdpub = WhoIs(NAME_TRAINCMDPUB);

	traincmd_receipt rcpt;
	rcpt.type = MSG_TRAINCMDRECEIPT;
	traincmd *cmd = &rcpt.cmd;

	for (;;) {
		int tid;
		int len = Receive(&tid, cmd, sizeof(traincmd));
		int replyresult = Reply(tid, NULL, 0);
		ASSERT(replyresult >= 0, "replyresult: %d", replyresult);
		ASSERT(len == sizeof(traincmd), "bad data");
		switch (cmd->name) {
			case SWITCH: {
				char swaddr = cmd->arg1;
				char swpos = cmd->arg2;
				int S = track_switchpos_straight(swpos);
				Putc(COM1, S ? TRAIN_SWITCH_STRAIGHT : TRAIN_SWITCH_CURVED, tid_com1);
				Putc(COM1, swaddr, tid_com1);
				Flush(tid_com1);
				rcpt.timestamp = Time(tid_time);
				publisher_pub(tid_traincmdpub, &rcpt, sizeof(rcpt));
				// @TODO: some delay might be needed here
				// Delay(TRAIN_PAUSE_SOLENOID, tid_time);
				// DelayUntil(rcpt.timestamp + TRAIN_PAUSE_SOLENOID, tid_time);
				Putc(COM1, TRAIN_SOLENOID_OFF, tid_com1);
				break;
			}
			default:
				ASSERT(0, "bad train cmd: %d", cmd->name);
				break;
		}
	}
}

int switchcmdrunner_new() {
	int tid_runner = Create(PRIORITY_SWITCHCMDRUNNER, switchcmdrunner);
	if (tid_runner < 0) return tid_runner;
	int tid_buffer = buffertask_new(NAME_SWITCHCMDRUNNER, PRIORITY_SWITCHCMDRUNNER, sizeof(traincmd));
	if (tid_buffer < 0) return tid_buffer;
	int tid_courier = courier_new(PRIORITY_SWITCHCMDRUNNER, tid_buffer, tid_runner, sizeof(traincmd), NAME_SWITCHCMDRUNNER_C);
	if (tid_courier < 0) return tid_courier;
	return tid_buffer;
}
