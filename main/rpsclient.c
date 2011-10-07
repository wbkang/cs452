#include <rpsclient.h>
#include <syscall.h>
#include <rawio.h>
#include <hardware.h>
#include <util.h>

void rps_client() {
	int server = WhoIs(RPS_SERVER_NAME);
	int move, rv;

	PRINT("Player %d: signing up", MyTid());
	rv = rps_signup(server);
	if (rv < 0) {
		PRINT("unabled to sign up");
		return;
	}

	PRINT("Player %d: starting to play", MyTid());

	for (int i = 0; i < 4; i++) {
		move = random() % 3;
		rv = rps_play(server, move);
		if (rv == RPS_RV_LOSS || rv == RPS_RV_WIN || rv == RPS_RV_TIE) {
			PRINT("Player %d: played %s and %s", MyTid(), move == RPS_MOVE_ROCK ? "ROCK" : move == RPS_MOVE_PAPER ? "PAPER" : "SCISSORS", rv == RPS_RV_LOSS ? "LOST" : rv == RPS_RV_WIN ? "WON" : "TIED");
		} else {
			PRINT("server responded with error %d", rv);
			break;
		}
	}

	PRINT("Player %d: exiting", MyTid());

	rps_quit(server);
}
