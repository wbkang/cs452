#include <constants.h>
#include <server/traincmdbuffer.h>
#include <server/buffertask.h>
#include <server/traincmdrunner.h>
#include <syscall.h>

int traincmdbuffer_new() {
	int tid = buffertask_new(NAME_TRAINCMDBUFFER, PRIORITY_TRAINCMDBUFFER, sizeof(traincmd));
	traincmdrunner_new();
	return tid;
}

int traincmdbuffer_put(int tid, traincmdname name, int arg1, int arg2) {
	traincmd cmd;
	cmd.type = TRAINCMD;
	cmd.name = name;
	cmd.arg1 = arg1;
	cmd.arg2 = arg2;
	cmd.timestamp = Time(WhoIs(NAME_TIMESERVER)); // @TODO: temp
	return buffertask_put(tid, &cmd, sizeof(traincmd));
}

int traincmdbuffer_get(int tid, traincmd *cmd) {
	return buffertask_get(tid, cmd, sizeof(traincmd));
}
