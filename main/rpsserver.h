#pragma once

#define RPS_SERVER_NAME "rp"

#define RPS_ERROR_BADREQNO -4
#define RPS_ERROR_NOTSIGNEDUP -5
#define RPS_ERROR_FULL -6
#define RPS_ERROR_OPPONENTQUIT -7
#define RPS_ERROR_BADDATA -8

#define RPS_MOVE_ROCK 0
#define RPS_MOVE_PAPER 1
#define RPS_MOVE_SCISSORS 2

#define RPS_RV_LOSS 1
#define RPS_RV_WIN 2
#define RPS_RV_TIE 3

void rps_server();

int rps_signup(int server);

int rps_play(int server, int move);

int rps_quit(int server);
