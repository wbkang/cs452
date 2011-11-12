#include <syscall.h>
#include <train.h>
#include <util.h>
#include <uconst.h>
#include <server/sensornotifier.h>

typedef struct {
	int tid_target;
} sensornotifier_args;

static void sensornotifier() {
	int tid;
	sensornotifier_args args;
	Receive(&tid, &args, sizeof(args));
	Reply(tid, NULL, 0);

	int tid_time = WhoIs(NAME_TIMESERVER);
	int tid_com1 = WhoIs(NAME_IOSERVER_COM1);
	//int tid_com2 = WhoIs(NAME_IOSERVER_COM2);
	int tid_traincmdbuf = WhoIs(NAME_TRAINCMDBUFFER);

	int modules[TRAIN_NUM_MODULES];
	for (int m = 0; m < TRAIN_NUM_MODULES; m++) {
		modules[m] = 0;
	}

	msg_sensor msg;
	msg.type = SENSOR;
	msg.module[1] = '\0';

	int last_ticks;
	int ticks = Time(tid_time) - MS2TICK(61); // average return time

	for (;;) {
		train_querysenmods(TRAIN_NUM_MODULES, tid_traincmdbuf); // @TODO: make this block
		last_ticks = ticks;
		ticks = Time(tid_time);
		// attribute the timestamp halfway between now and last query
		msg.timestamp = (last_ticks + ticks) >> 1;
		for (int m = 0; m < TRAIN_NUM_MODULES; m++) {
			int upper = Getc(COM1, tid_com1);
			int lower = Getc(COM1, tid_com1);
			int module = (upper << 8) | lower;
			int old_module = modules[m];
			modules[m] = module;
			int sensors = module ^ old_module;
			while (sensors) {
				int s = log2(sensors);
				ASSERT(s < 16, "bad s, module: %b, old_module: %b, sensors: %b", module, old_module, sensors);
				uint mask = 1 << s;
				msg.module[0] = 'A' + m;
				msg.id = 16 - s;
				msg.state = (module & mask) ? ON : OFF;
				Send(args.tid_target, &msg, sizeof(msg), NULL, 0);
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
