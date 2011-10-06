#include <rpsserver.h>
#include <syscall.h>
#include <rawio.h>
#include <hardware.h>

typedef struct _tag_rps_server_req {
	char no;
	char move;
} rps_server_req;

#define RPS_SERVER_SIGNUP 0
#define RPS_SERVER_PLAY 1
#define RPS_SERVER_QUIT 2

typedef struct _tag_rps_server_game {
	int player1;
	int player2;
} rps_server_game;

void rps_server() {
	PRINT("start");
	RegisterAs(RPS_SERVER_NAME);

	int player;
	rps_server_req req;
	int rv;

	for (;;) {
		if (sizeof(req) != Receive(&player, (void*) &req, sizeof(req))) {
			rv = RPS_ERROR_BADDATA;
		} else {
			PRINT("player: %d", player);
			switch (req.no) {
				case RPS_SERVER_SIGNUP:
					PRINT("signup");
					rv = 0x1;
					break;
				case RPS_SERVER_PLAY:
					PRINT("play %d", req.move);
					rv = 0x2;
					break;
				case RPS_SERVER_QUIT:
					PRINT("quit");
					rv = 0x3;
					break;
				default:
					rv = RPS_ERROR_BADREQNO;
					break;
			}
		}
		Reply(player, (void*) &rv, sizeof rv);
	}
}

int rps_send(int server, rps_server_req *req) {
	int rv;
	int len = Send(server, (void*) req, sizeof(rps_server_req), (void*) &rv, sizeof rv);
	if (len != sizeof rv) return RPS_ERROR_BADDATA;
	return rv;
}

int rps_signup(int server) {
	rps_server_req req;
	req.no = RPS_SERVER_SIGNUP;
	return rps_send(server, &req);
}

int rps_play(int server, char move) {
	rps_server_req req;
	req.no = RPS_SERVER_PLAY;
	req.move = move;
	return rps_send(server, &req);
}

int rps_quit(int server) {
	rps_server_req req;
	req.no = RPS_SERVER_QUIT;
	return rps_send(server, &req);
}
