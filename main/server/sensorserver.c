#include <syscall.h>
#include <train.h>
#include <util.h>
#include <uconst.h>
#include <server/sensorserver.h>
#include <server/buffertask.h>
#include <constants.h>
#include <server/comnotifier.h>
#include <server/courier.h>
#include <server/publisher.h>

typedef struct {
	int t;
	int modules[TRAIN_NUM_MODULES];
	int tid_pub;
	int tid_traincmdbuf;
	int i_byte;
	int i_mod;
	int high_byte;
	msg_sensor msg;
} state;

static void handle_comin(state *this, char msg[]) {
	msg_comin *comin = (msg_comin*) msg;
	if (this->i_byte == 0) {
		this->i_byte = 1;
		this->high_byte = comin->c;
	} else {
		this->i_byte = 0;
		int high_byte = this->high_byte;
		int low_byte = comin->c;
		int module = (high_byte << 8) | low_byte;
		int old_module = this->modules[this->i_mod];
		this->modules[this->i_mod] = module;
		int sensors = module ^ old_module;
		while (sensors) {
			int s = log2(sensors);
			uint mask = 1 << s;
			this->msg.module[0] = 'A' + this->i_mod;
			this->msg.id = 16 - s;
			this->msg.state = (module & mask) ? ON : OFF;
			buffertask_put(this->tid_pub, &this->msg, sizeof(msg_sensor));
			sensors &= ~mask;
		}
		this->i_mod++;
		if (this->i_mod == TRAIN_NUM_MODULES) {
			this->i_mod = 0;
			train_querysenmods(TRAIN_NUM_MODULES, this->tid_traincmdbuf);
		}
	}
}

static void handle_traincmdmsgreceipt(state *this, char msg[]) {
	traincmd_receipt *rcpt = (traincmd_receipt*) msg;
	traincmd *cmd = &rcpt->cmd;
	if (cmd->name == QUERY) {
		this->msg.timestamp = (this->t + rcpt->timestamp) >> 1;
		this->t = rcpt->timestamp;
	}
}

void sensorserver() {
	// init com1 notifier
	int tid_com1buffer = buffertask_new(NULL, 9, sizeof(msg_comin));
	comnotifier_new(tid_com1buffer, 9, COM2, WhoIs(NAME_IOSERVER_COM1));
	courier_new(9, tid_com1buffer, MyTid());

	// init state
	state this;
	this.t = 0;
	this.tid_pub = publisher_new(NAME_SENSORPUB, PRIORITY_SENSORPUB, sizeof(msg_sensor));
	this.tid_traincmdbuf = WhoIs(NAME_TRAINCMDBUFFER);
	for (int m = 0; m < TRAIN_NUM_MODULES; m++) {
		this.modules[m] = 0;
	}
	this.i_byte = 0;
	this.i_mod = 0;
	this.msg.type = SENSOR;
	this.msg.module[1] = '\0';

	int size_msg = max(sizeof(msg_comin), sizeof(traincmd));
	void* msg = malloc(size_msg);

	// initial query
	train_querysenmods(TRAIN_NUM_MODULES, this.tid_traincmdbuf);

	publisher_sub(WhoIs(NAME_TRAINCMDPUB), MyTid());

	for (;;) {
		int tid;
		Receive(&tid, msg, size_msg);
		Reply(tid, NULL, 0);
		msg_header *header = (msg_header*) msg;
		switch (header->type) {
			case COM_IN:
				handle_comin(&this, msg);
				break;
			case TRAINCMDRECEIPT:
				handle_traincmdmsgreceipt(&this, msg);
				break;
			default:
				break;
		}
	}
}

/*
 * API
 */

int sensorserver_new() {
	return Create(PRIORITY_SENSORSERVER, sensorserver);
}
