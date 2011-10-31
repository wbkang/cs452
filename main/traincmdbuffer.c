#include <traincmdbuffer.h>
#include <buffertask.h>
#include <constants.h>

int traincmdbuffer_new() {
	return buffertask_new(NAME_TRAINCMDBUFFER, PRIORITY_TRAINCMDBUFFER, sizeof(traincmd));
}

int traincmdbuffer_put(int tid, traincmdname name, int arg1, int arg2) {
	traincmd cmd;
	cmd.name = name;
	cmd.arg1 = arg1;
	cmd.arg2 = arg2;
	return buffertask_put(tid, &cmd, sizeof(traincmd));
}

int traincmdbuffer_get(int tid, traincmd *cmd) {
	return buffertask_get(tid, cmd, sizeof(traincmd));
}
