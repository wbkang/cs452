#include <sensornotifier.h>
#include <syscall.h>
#include <train.h>
#include <util.h>
#include <uconst.h>

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
	msg_sensor msg;
	msg.type = SENSOR;

	for (int m = 0; m < TRAIN_NUM_MODULES; m++) {
		modules[0] = 0;
	}

	for (;;) {
		train_querysenmods(TRAIN_NUM_MODULES, tid_traincmdbuf);
		//int start = uptime();
		for (int m = 0; m < TRAIN_NUM_MODULES; m++) {
			int upper = Getc(COM1, tid_com1);
			int lower = Getc(COM1, tid_com1);
			int module = (upper << 8) | lower;
			int old_module = modules[m];
			modules[m] = module;
			int sensors = (old_module ^ module) & (~old_module);
			while (sensors) {
				int s = log2(sensors);
				sensors &= ~(1 << s);
				msg.ticks = Time(tid_time);
				msg.module = 'A' + m;
				msg.id = 16 - s;
				Send(args.tid_target, &msg, sizeof(msg), NULL, 0);
			}
		}
		/*int end = uptime();
		int delta = end - start;
		char buf[32];
		sprintf(buf, "%d\n", delta);
		Putstr(COM2, buf, tid_com2);*/
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
