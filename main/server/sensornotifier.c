#include <syscall.h>
#include <train.h>
#include <util.h>
#include <uconst.h>
#include <server/sensornotifier.h>
#include <server/buffertask.h>

typedef struct {
	int tid_target;
} sensornotifier_args;

void sensornotifier() {
	int tid;
	sensornotifier_args args;
	Receive(&tid, &args, sizeof(args));
	Reply(tid, NULL, 0);

	int tid_time = WhoIs(NAME_TIMESERVER);
	int tid_com1 = WhoIs(NAME_IOSERVER_COM1);
	int tid_com2 = WhoIs(NAME_IOSERVER_COM2);
	int tid_traincmdbuf = WhoIs(NAME_TRAINCMDBUFFER);

	int modules[TRAIN_NUM_MODULES];
	for (int m = 0; m < TRAIN_NUM_MODULES; m++) {
		modules[m] = 0;
	}

	// init message
	const int size_msg = sizeof(msg_sensor);
	msg_sensor *msg = malloc(size_msg);
	msg->type = SENSOR;
	msg->module[1] = '\0';

	int timestamp_last;
	int timestamp = Time(tid_time) - MS2TICK(61); // average return time

	for (;;) {
		train_querysenmods(TRAIN_NUM_MODULES, tid_traincmdbuf); // @TODO: make this block
		timestamp_last = timestamp;
		timestamp = Time(tid_time);
		// attribute the timestamp halfway between now and last query
		msg->timestamp = (timestamp_last + timestamp) >> 1;
		char buf[256];
		sprintf(buf, "\x1B[s" "\x1B[2;56H" "sensor report: %dms        " "\x1B[u",
			TICK2MS(timestamp - timestamp_last)
		);
		Putstr(COM2, buf, tid_com2);
		for (int m = 0; m < TRAIN_NUM_MODULES; m++) {
			int upper = Getc(COM1, tid_com1);
			int lower = Getc(COM1, tid_com1);
			int module = (upper << 8) | lower;
			int old_module = modules[m];
			modules[m] = module;
			int sensors = module ^ old_module;
			while (sensors) {
				int s = log2(sensors);
				uint mask = 1 << s;
				msg->module[0] = 'A' + m;
				msg->id = 16 - s;
				msg->state = (module & mask) ? ON : OFF;
				Send(args.tid_target, msg, size_msg, NULL, 0);
				sensors &= ~mask;
			}
		}
	}
}

/*
 * API
 */

int sensornotifier_new(int tid_target) {
	sensornotifier_args args;
	args.tid_target = tid_target;
	int tid = Create(PRIORITY_SENSORNOTIFIER, sensornotifier);
	if (tid < 0) return tid;
	int rv = Send(tid, &args, sizeof(args), NULL, 0);
	if (rv < 0) return rv;
	return tid;
}
