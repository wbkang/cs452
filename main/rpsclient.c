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
		if (rv == 1 || rv == 2 || rv == 3) {
			PRINT("Player %d: played %s and %s", MyTid(), move == 0 ? "ROCK" : move == 1 ? "PAPER" : "SCISSORS", rv == 1 ? "LOST" : rv == 2 ? "WON" : "TIED");
		} else {
			PRINT("server responded with error %d", rv);
			break;
		}
	}

	PRINT("Player %d: exiting", MyTid());

	rps_quit(server);
}
