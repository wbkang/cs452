#pragma once

typedef enum {
	SPEED, REVERSE, SWITCH, SOLENOID,
	QUERY1, QUERY, GO, STOP, PAUSE
} traincmdname;

typedef struct {
	traincmdname name;
	int arg1;
	int arg2;
} traincmd;

int traincmdbuffer_new();
int traincmdbuffer_put(int tid, traincmdname name, int arg1, int arg2);
int traincmdbuffer_get(int tid, traincmd *cmd);
