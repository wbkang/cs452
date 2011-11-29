#include <constants.h>
#include <server/traincmdbuffer.h>
#include <server/buffertask.h>
#include <server/traincmdrunner.h>
#include <syscall.h>

int traincmdbuffer_new() {
	int tid = buffertask_new(NAME_TRAINCMDBUFFER, PRIORITY_TRAINCMDBUFFER, sizeof(traincmd));
	traincmdrunner_new();
	int tid2 = WhoIs(NAME_TRAINCMDBUFFER); // block until the buffer initialized
	ASSERT(tid == tid2, "there is an error in the nameserver, these two should be the same: %d %d", tid, tid2);
	return tid;
}

int traincmdbuffer_put(int tid, traincmdname name, int arg1, int arg2) {
	traincmd cmd;
	cmd.timestamp = Time(WhoIs(NAME_TIMESERVER)); // @TODO: remove or make more efficient
	cmd.type = MSG_TRAINCMD;
	cmd.name = name;
	cmd.arg1 = arg1;
	cmd.arg2 = arg2;
	return buffertask_put(tid, &cmd, sizeof(traincmd));
}

int traincmdbuffer_get(int tid, traincmd *cmd) {
	return buffertask_get(tid, cmd, sizeof(traincmd));
}
