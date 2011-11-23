#include <server/switchcmdrunner.h>
#include <syscall.h>
#include <train.h>
#include <uconst.h>
#include <server/publisher.h>
#include <server/buffertask.h>
#include <server/courier.h>

#include <console.h>
#include <ui/logstrip.h>

void switchcmdrunner() {
	int tid_com1 = WhoIs(NAME_IOSERVER_COM1);
	int tid_time = WhoIs(NAME_TIMESERVER);
	int tid_traincmdpub = WhoIs(NAME_TRAINCMDPUB);

	traincmd_receipt rcpt;
	rcpt.type = TRAINCMDRECEIPT;
	traincmd *cmd = &rcpt.cmd;



	console *con = console_new(COM2);
	logstrip *log = logstrip_new(con, 2, 80);



	for (;;) {
		int tid;
		// logstrip_printf(log, "receiving");
		int len = Receive(&tid, cmd, sizeof(traincmd));
		logstrip_printf(log, "received from %d", tid);
		Reply(tid, NULL, 0);
		ASSERT(len == sizeof(traincmd), "bad data");
		switch (cmd->name) {
			case SWITCH: {
				char swaddr = cmd->arg1;
				char swpos = cmd->arg2;
				// logstrip_printf(log, "got switch %d %c", swaddr, swpos);
				int S = track_switchpos_straight(swpos);
				Putc(COM1, S ? TRAIN_SWITCH_STRAIGHT : TRAIN_SWITCH_CURVED, tid_com1);
				Putc(COM1, swaddr, tid_com1);
				Flush(tid_com1);
				// logstrip_printf(log, "flushed");
				rcpt.timestamp = Time(tid_time);
				publisher_pub(tid_traincmdpub, &rcpt, sizeof(rcpt));
				// logstrip_printf(log, "published");
				// @TODO: some delay might be needed here
				// Delay(TRAIN_PAUSE_SOLENOID, tid_time);
				// DelayUntil(rcpt.timestamp + TRAIN_PAUSE_SOLENOID, tid_time);
				Putc(COM1, TRAIN_SOLENOID_OFF, tid_com1);
				break;
			}
			default:
				ERROR("bad train cmd: %d", cmd->name);
				break;
		}
	}
}

int switchcmdrunner_new(char name[]) {
	int tid_buffer = buffertask_new(name, PRIORITY_SWITCHCMDRUNNER, sizeof(traincmd));
	if (tid_buffer < 0) return tid_buffer;
	int tid = Create(PRIORITY_SWITCHCMDRUNNER, switchcmdrunner);
	if (tid < 0) return tid;
	int tid_courier = courier_new(PRIORITY_SWITCHCMDRUNNER, tid_buffer, tid);
	if (tid_courier < 0) return tid_courier;
	return tid_buffer;
}
