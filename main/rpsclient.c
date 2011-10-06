#include <rpsclient.h>
#include <syscall.h>
#include <rawio.h>
#include <hardware.h>

void rps_client() {
	PRINT("starting... my tid: %d", MyTid());
	int server = WhoIs(RPS_SERVER_NAME);
	PRINT("got rps tid %d", server);
	int rv;

	PRINT("signing up...");
	rv = rps_signup(server);
	PRINT("server replied: %d", rv);

	rv = rps_play(server, RPS_MOVE_ROCK);
	PRINT("played: %d", rv);
	rv = rps_play(server, RPS_MOVE_PAPER);
	PRINT("played: %d", rv);
	rv = rps_play(server, RPS_MOVE_SCISSORS);
	PRINT("played: %d", rv);

	PRINT("quitting");
	rps_quit(server);
}
