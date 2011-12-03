#pragma once

#include <ui/logdisplay.h>
#include <ui/logstrip.h>
#include <ui/timedisplay.h>
#include <ui/cmdline.h>
#include <track_node.h>

#define LEN_SENSOR_HIST 8

typedef struct a0ui a0ui;
struct train;

a0ui* a0ui_new(cmdprocessor cmdproc, char track);
void a0ui_on_sensor(a0ui *this, char module, int id, int senstate);
void a0ui_on_time(void* vthis, void* vtick);
void a0ui_on_reverse(a0ui *this, int train, int t);
void a0ui_on_switch(a0ui *this, char no, char pos, int t);
void a0ui_on_cmdlog(a0ui *this, char *msg);
#define a0ui_on_cmdlogf(this, ...) { char __buf[256]; sprintf(__buf, __VA_ARGS__); a0ui_on_cmdlog(this, __buf); }
void a0ui_on_log(a0ui *this, char *msg);
#define a0ui_on_logf(this, ...) { char __buf[256]; sprintf(__buf, __VA_ARGS__); a0ui_on_log(this, __buf); }
void a0ui_on_train_location(a0ui *this, struct train* train);
void a0ui_on_train_location_log(a0ui *this, char *log);
#define a0ui_on_train_location_logf(this, ...) { char __buf[256]; sprintf(__buf, __VA_ARGS__); a0ui_on_train_location_log(this, __buf); }
void a0ui_on_location_attr(a0ui *this, track_node *sensor, int trainno);
void a0ui_on_location_attr_log(a0ui *this, char *log);
#define a0ui_on_location_attr_logf(this, ...) { char __buf[256]; sprintf(__buf, __VA_ARGS__); a0ui_on_location_attr_log(this, __buf); }
void a0ui_on_trip_log(a0ui *this, char *log);
#define a0ui_on_trip_logf(this, ...) { char __buf[256]; sprintf(__buf, __VA_ARGS__); a0ui_on_trip_log(this, __buf); }
void a0ui_on_key_input(a0ui *this, char c, void *that);
void a0ui_on_cmdreset(a0ui *this);
void a0ui_on_quit(a0ui *this);

