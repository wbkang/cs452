#include <rpsclient.h>
#include <syscall.h>
#include <rawio.h>
#include <hardware.h>
#include <util.h>

void rps_client() {
	int server = WhoIs(RPS_SERVER_NAME);
	int move, rv;

	rv = rps_signup(server);
	if (rv < 0) {
		PRINT("unabled to sign up");
		return;
	}

	for (int i = 0; i < 4; i++) {
		move = random() % 3;
		rv = rps_play(server, move);
		if (rv == 1 || rv == 2 || rv == 3) {
			PRINT("player %d: played: %s, got: %s", MyTid(), move == 0 ? "ROCK" : move == 1 ? "PAPER" : "SCISSORS", rv == 1 ? "LOSS" : rv == 2 ? "WIN" : "TIE");
		} else {
			PRINT("server responded with error %d", rv);
			break;
		}
	}

	rps_quit(server);
}
