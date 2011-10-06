#pragma once

#define RPS_SERVER_NAME "rp"

#define RPS_ERROR_BADDATA -3
#define RPS_ERROR_BADREQNO -4

#define RPS_MOVE_ROCK 0
#define RPS_MOVE_PAPER 1
#define RPS_MOVE_SCISSORS 2

void rps_server();

int rps_signup(int server);

int rps_play(int server, char RPS);

int rps_quit(int server);
