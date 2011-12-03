#include <ui/a0ui.h>
#include <task/a0.h>
#include <syscall.h>
#include <train.h>
#include <server/uiserver.h>

#define SIDE_LOG_COL 56
#define MAX_KNOWN_TRAIN 5
#define TRAIN_INFOLOG_COUNT 3

struct a0ui {
	ui_id id_ui;
	logdisplay *log;
	logdisplay *trainloclog;
	logdisplay *locattrlog;
	logdisplay *triplog;
	logstrip *cmdlog;
	logstrip *sensorlog;
	logstrip *train1info[3];
	logstrip *train2info[3];
	timedisplay *timedisplay;
	track_template *template;
	cmdline *cmdline;
	// for sensor history TODO kind of temporary
	char hist_mod[LEN_SENSOR_HIST];
	int hist_id[LEN_SENSOR_HIST];
	// multi-train info support
	int train_info_start_line;
	int known_train_count;
	int known_train_map[TRAIN_MAX_TRAIN_ADDR];
	logstrip **traininfolog[MAX_KNOWN_TRAIN];
};


#define CONSOLE_DUMP_LINE (CONSOLE_CMD_LINE + 4)
#define CONSOLE_DUMP_COL 1

a0ui* a0ui_new(cmdprocessor cmdproc, char track) {
	a0ui *this = malloc(sizeof(a0ui));
	this->id_ui = uiserver_register();
	this->template = track_template_new(track);
	this->cmdlog = logstrip_new(CONSOLE_LOG_LINE, CONSOLE_LOG_COL, 100);
	this->cmdline = cmdline_new(CONSOLE_CMD_LINE, CONSOLE_CMD_COL, cmdproc);
	this->sensorlog = logstrip_new(CONSOLE_SENSOR_LINE, CONSOLE_SENSOR_COL, 100);
	this->log = logdisplay_new(CONSOLE_DUMP_LINE, CONSOLE_DUMP_COL, 19, 55, ROUNDROBIN, "log");
	this->trainloclog = logdisplay_new(8, SIDE_LOG_COL, 8, 100, ROUNDROBIN, "train location log");
	this->locattrlog = logdisplay_new(8 + 9, SIDE_LOG_COL, 8, 100, ROUNDROBIN, "location attribution log");
	this->triplog = logdisplay_new(8 + 9 + 9, SIDE_LOG_COL, 8, 100, ROUNDROBIN, "engineer triplog");
	this->train_info_start_line = 2;
	this->known_train_count = 0;
	for (int i = 0; i < TRAIN_MAX_TRAIN_ADDR; i++) {
		this->known_train_map[i] = -1;
	}
	this->timedisplay = timedisplay_new(1, 1);
	cmdline_clear(this->cmdline);
	for (int i = 0; i < LEN_SENSOR_HIST; i++) {
		this->hist_mod[i] = 0;
		this->hist_id[i] = 0;
	}
	// we have to cache these logstrips because we can't initialize them
	// while a0 is receiving messages.
	for (int i = 0; i < MAX_KNOWN_TRAIN; i++) {
		this->traininfolog[i] = malloc(sizeof(logstrip*) * TRAIN_INFOLOG_COUNT);
		for (int k = 0; k < TRAIN_INFOLOG_COUNT; k++) {
			this->traininfolog[i][k] = logstrip_new(this->train_info_start_line + i * TRAIN_INFOLOG_COUNT + k, SIDE_LOG_COL + 2, 100);
		}
	}
	return this;
}

void a0ui_on_sensor(a0ui *this, char module, int id, int senstate) {
	char *hist_mod = this->hist_mod;
	int *hist_id = this->hist_id;

	if (senstate == OFF) return;
	for (int i = LEN_SENSOR_HIST - 1; i > 0; i--) {
		hist_mod[i] = hist_mod[i - 1];
		hist_id[i] = hist_id[i - 1];
	}
	hist_mod[0] = module;
	hist_id[0] = id;

	int max_hist_idx = 0;
	char buf[1024], *p = buf;
	for (int i = 0; (i < LEN_SENSOR_HIST) && hist_mod[i]; i++) {
		p += sprintf(p, "%c%d, ", hist_mod[i], hist_id[i]);
		if (i == LEN_SENSOR_HIST) {
			p += sprintf(p, "...");
		}
		max_hist_idx = i;
	}
	logstrip_printf(this->sensorlog, buf);
}

void a0ui_on_time(void* vthis, void* vtick) {
	a0ui *this = vthis;
	timedisplay_update(this->timedisplay, (int)vtick);
}

void a0ui_on_reverse(a0ui *this, int train, int t) {
	logdisplay_printf(this->log, "[%7d] reversed train %d", t, train);
	logdisplay_flushline(this->log);
}

void a0ui_on_switch(a0ui *this, char no, char pos, int t) {
	track_template_updateswitch(this->template, no, pos);
}


void a0ui_on_quit(a0ui *this) {
	logstrip_printf(this->cmdlog, "quitting...");
	uiserver_movecursor(this->id_ui, CONSOLE_CMD_LINE + 1, 1);
	uiserver_force_refresh(this->id_ui);
}

void a0ui_on_cmdlog(a0ui *this, char *msg) {
	logstrip_printf(this->cmdlog, msg);
}

void a0ui_on_log(a0ui *this, char *msg) {
	logdisplay_puts(this->log, msg);
	logdisplay_flushline(this->log);
}

void a0ui_on_train_location(a0ui *this, train* train) {
	ASSERT((int)this % 4 == 0, "a0ui unaligned");
	ASSERT((int)train % 4 == 0, "train unaligned");
	int idx = this->known_train_map[train->no];
	if (idx == -1) {
		if (train_is_lost(train)) {
			return;
		}
		idx = this->known_train_count++;
		this->known_train_map[train->no] = idx;
	}

	location loc = train_get_frontloc(train);

	char *direction_str;
	switch (train_get_dir(train)) {
		case TRAIN_FORWARD:
			direction_str = ">";
			break;
		case TRAIN_BACKWARD:
			direction_str = "<";
			break;
		default:
			direction_str = "?";
			break;
	}

	location dest = train->destination;

	logstrip_printf(this->traininfolog[idx][0],
		"train %d at %L heading %s at %dmm/s (%dmm/s^2 -> %dmm/s^2) to %L",
		train->no,
		&loc,
		direction_str,
		(int) (train_get_velocity(train) * 1000),
		(int) (train->a_i * 1000 * 1000),
		(int) (train->a * 1000 * 1000),
		&dest
	);

	char msg[512];
	char *b;

	b = msg;
	b += sprintf(b, "\tres:");
	for (int i = 0; i < train->reservation->len; i++) {
		track_edge *edge = train->reservation->edges[i];
		b += sprintf(b, " %s-%s", edge->src->name, edge->dest->name);
	}
	logstrip_printf(this->traininfolog[idx][1], msg);

	b = msg;
	b += sprintf(b, "\talt:");
	for (int i = 0; i < train->reservation_alt->len; i++) {
		track_edge *edge = train->reservation->edges[i];
		b += sprintf(b, " %s-%s", edge->src->name, edge->dest->name);
	}
	logstrip_printf(this->traininfolog[idx][2], msg);
}

void a0ui_on_key_input(a0ui *this, char c, void *that) {
	cmdline_handleinput(this->cmdline, c, that);
}

void a0ui_on_cmdreset(a0ui *this) {
	cmdline_clear(this->cmdline);
}

void a0ui_on_train_location_log(a0ui *this, char *log) {
	logdisplay_puts(this->trainloclog, log);
	logdisplay_flushline(this->trainloclog);
}

void a0ui_on_location_attr_log(a0ui *this, char *log) {
	logdisplay_puts(this->locattrlog, log);
	logdisplay_flushline(this->locattrlog);
}

void a0ui_on_trip_log(a0ui *this, char *log) {
	logdisplay_puts(this->triplog, log);
	logdisplay_flushline(this->triplog);
}

void a0ui_on_location_attr(a0ui *this, track_node *sensor, int trainno) {
	char *p = (char*) sensor->name;
	char mod[3];
	p += strgetw(p, mod, 3);
	int id = strgetui(&p);
	int trainidx = this->known_train_map[trainno];
	if (trainidx != -1) {
		track_template_updatesensor(this->template, mod[0], id, trainidx);
	}
}
