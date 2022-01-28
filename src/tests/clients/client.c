#include <dlfcn.h>
#include "tests/clients/client.h"
#include "graphf.h"
#include "board.h"
#include <string.h>
#include <stdio.h>
#include <unistd.h>

int test_get_player_name(struct client* client)
{
	char const* name = client->get_player_name();
	int is_valid = 1;

	for (size_t i = 0; i < strlen(name) - 1; i++) {
		if (('a' > name[i] || name[i] > 'z') && ('A' > name[i] || name[i] > 'Z') && ('0' > name[i] || name[i] > '9') && (name[i] != '_')
			&& (name[i] != '-') && (name[i] != ' ')) {
			is_valid = 0;
			break;
		}
	}

	return is_valid;
}

int test_initialize(struct client* client)
{
	struct graph_t* g = graph__square(10);
	struct graph_t* g_beg = graph__square(10);

	size_t num_walls = 10;
	client->initialize(WHITE, g, num_walls);

	struct board_t* b = (struct board_t*)dlsym(client->library, "b");
	struct board_t b_check = {g, g_beg, {num_walls, num_walls}, {SIZE_MAX, SIZE_MAX}};

	int is_valid = ((b->graph == g) && board__are_equal(b, &b_check));

	client->finalize();

	graph__free(g_beg);

	return is_valid;
}

int test_play(struct client* client)
{
	int is_valid = 1;

	struct graph_t* g = graph__square(10);
	struct graph_t* g_check = graph__square(10);
	struct graph_t* g_check_beg = graph__square(10);

	size_t num_walls = 10;
	client->initialize(WHITE, g, num_walls);

	struct board_t* b = (struct board_t*)dlsym(client->library, "b");
	struct board_t b_check = {g_check, g_check_beg, {num_walls, num_walls}, {1, SIZE_MAX}};

	struct move_t move = client->play((struct move_t) {1, {{-1, -1}, {-1, -1}}, MOVE, BLACK});
	if (!board__check_move(&b_check, &move) || move.c != WHITE || move.t != MOVE) {
		printf("      Client sent a bad first move.\n");
		is_valid = 0;
	}
	board__apply_move(&b_check, &move);

	if (!board__are_equal(b, &b_check)) {
		printf("      Client didn't update its board copy well for first move.\n");
		is_valid = 0;
	}

	b_check.pos[BLACK] = 2;

	move = client->play((struct move_t) {2, {{-1, -1}, {-1, -1}}, MOVE, BLACK});
	if (!board__check_move(&b_check, &move) || move.c != WHITE) {
		printf("      Client sent a bad second move.\n");
		is_valid = 0;
	}
	board__apply_move(&b_check, &move);

	if (!board__are_equal(b, &b_check)) {
		printf("      Client didn't update its board copy well for second move.\n");
		is_valid = 0;
	}

	client->finalize();
	graph__free(g_check);
	graph__free(g_check_beg);
	return is_valid;
}
