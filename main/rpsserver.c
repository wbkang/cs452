#include <rpsserver.h>
#include <syscall.h>
#include <rawio.h>
#include <hardware.h>

typedef struct _tag_rps_server_req {
	char no;
	int move;
} rps_server_req;

typedef struct _tag_rps_server_game {
	char state;
	int player[2];
	int move[2];
} rps_server_game;

#define RPS_SERVER_SIGNUP 0
#define RPS_SERVER_PLAY 1
#define RPS_SERVER_QUIT 2

#define RPS_GAME_GOOD 0
#define RPS_GAME_DEAD 1

#define RPS_SERVER_MAX_GAMES 16

int rps_find_player(rps_server_game *game, int player, int *g, int *p) {
	for (*g = 0; *g < RPS_SERVER_MAX_GAMES; (*g)++) {
		for (*p = 0; *p < 2; (*p)++) {
			if (game[*g].player[*p] == player) return 0;
		}
	}
	return RPS_ERROR_NOTSIGNEDUP;
}

int rps_player_exists(rps_server_game *game, int player) {
	int g, p;
	return rps_find_player(game, player, &g, &p) == 0;
}

int rps_do_signup(rps_server_game *game, int player) {
	if (rps_player_exists(game, player)) return 0; // already in a game
	int rv = 0;
	for (int i = 0; i < RPS_SERVER_MAX_GAMES; i += 1) {
		if (game[i].state == RPS_GAME_GOOD && game[i].player[0] != -1 && game[i].player[1] == -1) {
			Reply(game[i].player[0], (void*) &rv, sizeof rv); // unblock player1
			game[i].player[1] = player;
			return 0; // unblock player2
		}
	}
	for (int i = 0; i < RPS_SERVER_MAX_GAMES; i += 1) {
		if (game[i].state == RPS_GAME_GOOD && game[i].player[0] == -1) {
			game[i].player[0] = player;
			return 1;
		}
	}
	return RPS_ERROR_FULL;
}

int rps_do_play(rps_server_game *game, int player, int move) {
	static int rps_outcome[3][3] = {{3, 1, 2}, {2, 3, 1}, {1, 2, 3}};
	int g, p;
	int rv = rps_find_player(game, player, &g, &p);
	if (rv < 0) return rv;
	if (game[g].state != RPS_GAME_GOOD) return RPS_ERROR_OPPONENTQUIT;
	game[g].move[p] = move;
	if (game[g].move[p ^ 1] == -1) return 1; // wait for the other player
	for (int i = 0; i < 2; i++) {
		rv = rps_outcome[game[g].move[i]][game[g].move[i ^ 1]];
		Reply(game[g].player[i], (void*) &rv, sizeof rv);
	}
	for (int i = 0; i < 2; i++) game[g].move[i] = -1;
	bwgetc(COM2); // pause
	return 1; // do not return
}

int rps_do_quit(rps_server_game *game, int player) {
	int g, p;
	int rv = rps_find_player(game, player, &g, &p);
	if (rv < 0) return rv;
	game[g].player[p] = -1;
	game[g].move[p] = -1;
	game[g].state = (game[g].player[p ^ 1] == -1) ? RPS_GAME_GOOD : RPS_GAME_DEAD;
	return 0;
}

void rps_server() {
	RegisterAs(RPS_SERVER_NAME);
	rps_server_game games[RPS_SERVER_MAX_GAMES];
	int player;
	rps_server_req req;
	int rv;
	// initialize games
	for (int g = 0; g < RPS_SERVER_MAX_GAMES; g++) {
		games[g].state = RPS_GAME_GOOD;
		for (int p = 0; p < 2; p++) {
			games[g].player[p] = -1;
			games[g].move[p] = -1;
		}
	}
	for (;;) {
		rv = 0;
		int msglen = Receive(&player, (void*) &req, sizeof(req));
		if (msglen == sizeof(req)) {
			rv = RPS_ERROR_BADDATA;
		} else {
			switch (req.no) {
				case RPS_SERVER_SIGNUP:
					rv = rps_do_signup(games, player);
					break;
				case RPS_SERVER_PLAY:
					rv = rps_do_play(games, player, req.move);
					break;
				case RPS_SERVER_QUIT:
					rv = rps_do_quit(games, player);
					break;
				default:
					rv = RPS_ERROR_BADREQNO;
					break;
			}
		}
		if (rv <= 0) Reply(player, (void*) &rv, sizeof rv); // return errors
	}
}

inline int rps_send(int server, rps_server_req *req) {
	int rv;
	int len = Send(server, (void*) req, sizeof(rps_server_req), (void*) &rv, sizeof rv);
	if (len != sizeof rv) return RPS_ERROR_BADDATA;
	return rv;
}

/*
 * API
 */

int rps_signup(int server) {
	rps_server_req req;
	req.no = RPS_SERVER_SIGNUP;
	return rps_send(server, &req);
}

int rps_play(int server, int move) {
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
