#pragma once

typedef enum {
	SPEED, REVERSE, SWITCH, SOLENOID,
	QUERY1, QUERY, GO, STOP, PAUSE
} traincmdname;

typedef struct {
	traincmdname name;
	char byte1;
	char byte2;
} traincmd;

int traincmdbuffer_new();
int traincmdbuffer_put(int tid, traincmdname name, char byte1, char byte2);
int traincmdbuffer_get(int tid, traincmd *cmd);
