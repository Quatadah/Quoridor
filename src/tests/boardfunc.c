#include <stdio.h>
#include <stdlib.h>
#include "board.h"
#include "move.h"

struct board_t init_board()
{
	return (struct board_t) {graph__square(10), graph__square(10), {10, 10}, {99, 0}};
}

// type = NO_TYPE
int test_board__apply_move_no_type()
{
	struct board_t b = init_board();
	struct board_t b_check = init_board();

	struct move_t move = {rand(), {{rand(), rand()}, {rand(), rand()}}, NO_TYPE, rand() % 3};
	board__apply_move(&b, &move);

	int is_valid = board__are_equal(&b, &b_check);

	board__release(&b);
	board__release(&b_check);

	return is_valid;
}

// type = WALL
int test_board__apply_move_wall()
{
	struct board_t b = init_board();
	struct board_t b_check = init_board();

	b_check.num_walls[BLACK]--;
	gsl_spmatrix_uint_set(b_check.graph->t, 0, 10, NONE);
	gsl_spmatrix_uint_set(b_check.graph->t, 10, 0, NONE);
	gsl_spmatrix_uint_set(b_check.graph->t, 1, 11, NONE);
	gsl_spmatrix_uint_set(b_check.graph->t, 11, 1, NONE);

	struct move_t move = {rand(), {{0, 10}, {1, 11}}, WALL, BLACK};

	int is_valid = board__apply_move(&b, &move) && board__are_equal(&b, &b_check);

	board__release(&b);
	board__release(&b_check);

	return is_valid;
}

// type = MOVE
int test_board__apply_move_move()
{
	struct board_t b = init_board();
	struct board_t b_check = init_board();

	b_check.pos[BLACK] = 98;

	struct move_t move = {98, {{rand(), rand()}, {rand(), rand()}}, MOVE, BLACK};

	int is_valid = board__apply_move(&b, &move) && board__are_equal(&b, &b_check);

	board__release(&b);
	board__release(&b_check);

	return is_valid;
}

int test_board__check_move_move()
{
	struct board_t b = init_board();
	int is_valid = 1;

	struct move_t move = {SIZE_MAX, {{rand(), rand()}, {rand(), rand()}}, MOVE, BLACK};
	if (board__check_move(&b, &move)) {
		printf("   Wrong vertex id accepted.\n");
		is_valid = 0;
	}

	move.m = b.pos[BLACK];
	if (board__check_move(&b, &move)) {
		printf("   Can move on BLACK's position.\n");
		is_valid = 0;
	}

	move.m = b.pos[WHITE];
	if (board__check_move(&b, &move)) {
		printf("   Can move on WHITE's position.\n");
		is_valid = 0;
	}

	b.pos[WHITE] = SIZE_MAX;
	b.pos[BLACK] = SIZE_MAX;
	move.m = rand() % 10;
	if (!board__check_move(&b, &move)) {
		printf("    Can't place on start position at start.\n");
		is_valid = 0;
	}

	move.m = rand() % 90;
	if (board__check_move(&b, &move)) {
		printf("    Can move on a non starting position at start.\n");
		is_valid = 0;
	}

	b.pos[WHITE] = 0;
	b.pos[BLACK] = 55;
	size_t positions[4] = {54, 56, 45, 65};
	for (int i = 0; i < 4; i++) {
		move.m = positions[i];
		if (!board__check_move(&b, &move)) {
			printf("    Can't move nearly %zu.\n", positions[i]);
			is_valid = 0;
		}
	}
	move.m = 1 + rand() % 44;
	if (board__check_move(&b, &move)) {
		printf("   Can move too freely 1.\n");
		is_valid = 0;
	}
	move.m = 66 + rand() % (100 - 66);
	if (board__check_move(&b, &move)) {
		printf("   Can move too freely 2.\n");
		is_valid = 0;
	}

	size_t possible_positions[] = {53, 44, 64, 57, 46, 66, 35, 44, 46, 75, 66, 64};
	for (int i = 0; i < 4; i++) {
		b.pos[WHITE] = positions[i];
		move.m = possible_positions[3 * i];
		if (!board__check_move(&b, &move)) {
			printf("   Can't jump a player correctly %zu.\n", positions[i]);
			is_valid = 1;
		}
		move.m = possible_positions[3 * i + 1];
		if (board__check_move(&b, &move)) {
			printf("   Can jump a player and turn while normally not possible %zu %zu.\n", positions[i], possible_positions[3 * i + 1]);
			is_valid = 1;
		}
		move.m = possible_positions[3 * i + 2];
		if (board__check_move(&b, &move)) {
			printf("   Can jump a player and turn while normally not possible %zu %zu.\n", positions[i], possible_positions[3 * i + 2]);
			is_valid = 1;
		}

		enum dir_t edge_direction = graph__get_edge(b.graph, (struct edge_t) {positions[i], possible_positions[3 * i]});
		graph__set_edge(b.graph, (struct edge_t) {positions[i], possible_positions[3 * i]}, 0);

		move.m = possible_positions[3 * i];
		if (board__check_move(&b, &move)) {
			printf("   Can jump a player without turning while not possible %zu.\n", positions[i]);
			is_valid = 1;
		}
		move.m = possible_positions[3 * i + 1];
		if (!board__check_move(&b, &move)) {
			printf("   Can't jump a player and turn while normally possible %zu %zu.\n", positions[i], possible_positions[3 * i + 1]);
			is_valid = 1;
		}
		move.m = possible_positions[3 * i + 2];
		if (!board__check_move(&b, &move)) {
			printf("   Can't jump a player and turn while normally possible %zu %zu.\n", positions[i], possible_positions[3 * i + 2]);
			is_valid = 1;
		}

		graph__set_edge(b.graph, (struct edge_t) {positions[i], possible_positions[3 * i]}, edge_direction);
	}

	board__release(&b);

	return is_valid;
}

int test_board__check_move_wall()
{
	struct board_t b = init_board();
	int is_valid = 1;

	struct move_t move = {rand(), {{0, 0}, {0, 0}}, WALL, BLACK};
	struct edge_t edges1[4][2] = {{{45, 46}, {55, 56}}, {{46, 45}, {55, 56}}, {{45, 46}, {56, 55}}, {{46, 45}, {56, 55}}};
	for (int i = 0; i < 4; i++) {
		move.e[0] = edges1[i][0];
		move.e[1] = edges1[i][1];
		if (!board__check_move(&b, &move)) {
			printf("   Can't place a good wall vertically %zu %zu %zu %zu.\n",
				   edges1[i][0].fr,
				   edges1[i][0].to,
				   edges1[i][1].fr,
				   edges1[i][1].to);
			is_valid = 0;
		}
	}

	struct edge_t edges2[4][2] = {{{45, 55}, {46, 56}}, {{55, 45}, {46, 56}}, {{45, 55}, {56, 46}}, {{55, 45}, {56, 46}}};
	for (int i = 0; i < 4; i++) {
		move.e[0] = edges2[i][0];
		move.e[1] = edges2[i][1];
		if (!board__check_move(&b, &move)) {
			printf("   Can't place a good wall horitontally %zu %zu %zu %zu.\n",
				   edges2[i][0].fr,
				   edges2[i][0].to,
				   edges2[i][1].fr,
				   edges2[i][1].to);
			is_valid = 0;
		}
	}

	struct edge_t edges3[2][2] = {{{45, 78}, {46, 56}}, {{55, 45}, {46, 98}}};
	for (int i = 0; i < 2; i++) {
		move.e[0] = edges3[i][0];
		move.e[1] = edges3[i][1];
		if (board__check_move(&b, &move)) {
			printf("   Can place an inconsistant wall %zu %zu %zu %zu.\n",
				   edges3[i][0].fr,
				   edges3[i][0].to,
				   edges3[i][1].fr,
				   edges3[i][1].to);
			is_valid = 0;
		}
	}

	move.e[0] = (struct edge_t) {45, 46};
	move.e[1] = (struct edge_t) {55, 56};
	b.num_walls[0] = 0;
	if (board__check_move(&b, &move)) {
		printf("   Can place a wall without available wall.\n");
		is_valid = 0;
	}

	b.num_walls[0] = 10;
	for (int i = 0; i < 4; i++) {
		move.e[0] = edges1[i][0];
		move.e[1] = edges1[i][1];
		enum dir_t dir1 = graph__get_edge(b.graph, (struct edge_t) {edges1[i][0].fr, edges1[i][1].fr});
		enum dir_t dir2 = graph__get_edge(b.graph, (struct edge_t) {edges1[i][0].to, edges1[i][1].fr});
		enum dir_t dir3 = graph__get_edge(b.graph, (struct edge_t) {edges1[i][0].fr, edges1[i][1].to});
		enum dir_t dir4 = graph__get_edge(b.graph, (struct edge_t) {edges1[i][0].to, edges1[i][1].to});
		graph__set_edge(b.graph, (struct edge_t) {edges1[i][0].fr, edges1[i][1].fr}, NONE);
		graph__set_edge(b.graph, (struct edge_t) {edges1[i][0].to, edges1[i][1].fr}, NONE);
		graph__set_edge(b.graph, (struct edge_t) {edges1[i][0].fr, edges1[i][1].to}, NONE);
		graph__set_edge(b.graph, (struct edge_t) {edges1[i][0].to, edges1[i][1].to}, NONE);
		if (board__check_move(&b, &move)) {
			printf("   Can place a wall without side edge %zu %zu %zu %zu.\n",
				   edges1[i][0].fr,
				   edges1[i][0].to,
				   edges1[i][1].fr,
				   edges1[i][1].to);
			is_valid = 0;
		}
		graph__set_edge(b.graph, (struct edge_t) {edges1[i][0].fr, edges1[i][1].fr}, dir1);
		graph__set_edge(b.graph, (struct edge_t) {edges1[i][0].to, edges1[i][1].fr}, dir2);
		graph__set_edge(b.graph, (struct edge_t) {edges1[i][0].fr, edges1[i][1].to}, dir3);
		graph__set_edge(b.graph, (struct edge_t) {edges1[i][0].to, edges1[i][1].to}, dir4);
	}

	for (int i = 0; i < 4; i++) {
		move.e[0] = edges2[i][0];
		move.e[1] = edges2[i][1];
		enum dir_t dir1 = graph__get_edge(b.graph, (struct edge_t) {edges2[i][0].fr, edges2[i][1].fr});
		enum dir_t dir2 = graph__get_edge(b.graph, (struct edge_t) {edges2[i][0].to, edges2[i][1].fr});
		enum dir_t dir3 = graph__get_edge(b.graph, (struct edge_t) {edges2[i][0].fr, edges2[i][1].to});
		enum dir_t dir4 = graph__get_edge(b.graph, (struct edge_t) {edges2[i][0].to, edges2[i][1].to});
		graph__set_edge(b.graph, (struct edge_t) {edges2[i][0].fr, edges2[i][1].fr}, NONE);
		graph__set_edge(b.graph, (struct edge_t) {edges2[i][0].to, edges2[i][1].fr}, NONE);
		graph__set_edge(b.graph, (struct edge_t) {edges2[i][0].fr, edges2[i][1].to}, NONE);
		graph__set_edge(b.graph, (struct edge_t) {edges2[i][0].to, edges2[i][1].to}, NONE);
		if (board__check_move(&b, &move)) {
			printf("   Can place a wall without side edge %zu %zu %zu %zu.\n",
				   edges2[i][0].fr,
				   edges2[i][0].to,
				   edges2[i][1].fr,
				   edges2[i][1].to);
			is_valid = 0;
		}
		graph__set_edge(b.graph, (struct edge_t) {edges2[i][0].fr, edges2[i][1].fr}, dir1);
		graph__set_edge(b.graph, (struct edge_t) {edges2[i][0].to, edges2[i][1].fr}, dir2);
		graph__set_edge(b.graph, (struct edge_t) {edges2[i][0].fr, edges2[i][1].to}, dir3);
		graph__set_edge(b.graph, (struct edge_t) {edges2[i][0].to, edges2[i][1].to}, dir4);
	}

	enum dir_t dir1 = graph__get_edge(b.graph, (struct edge_t) {1, 11});
	enum dir_t dir2 = graph__get_edge(b.graph, (struct edge_t) {2, 12});
	enum dir_t dir3 = graph__get_edge(b.graph, (struct edge_t) {3, 13});
	enum dir_t dir4 = graph__get_edge(b.graph, (struct edge_t) {4, 14});
	graph__set_edge(b.graph, (struct edge_t) {1, 11}, NONE);
	graph__set_edge(b.graph, (struct edge_t) {2, 12}, NONE);
	graph__set_edge(b.graph, (struct edge_t) {3, 13}, NONE);
	graph__set_edge(b.graph, (struct edge_t) {4, 14}, NONE);
	struct edge_t edges4[8][2] = {{{2, 3}, {12, 13}},
								  {{2, 3}, {13, 12}},
								  {{3, 2}, {13, 12}},
								  {{3, 2}, {12, 13}},
								  {{12, 13}, {2, 3}},
								  {{12, 13}, {3, 2}},
								  {{13, 12}, {3, 2}},
								  {{13, 12}, {2, 3}}};
	for (int i = 0; i < 8; i++) {
		move.e[0] = edges4[i][0];
		move.e[1] = edges4[i][1];
		if (!board__check_move(&b, &move)) {
			printf("   Wall between walls not possible %zu %zu %zu %zu.\n",
				   edges4[i][0].fr,
				   edges4[i][0].to,
				   edges4[i][1].fr,
				   edges4[i][1].to);
			is_valid = 0;
		}
	}
	move.e[0] = (struct edge_t) {1, 2};
	move.e[1] = (struct edge_t) {11, 12};
	if (board__check_move(&b, &move))
		printf("   Cross wall on wall possible\n");
	graph__set_edge(b.graph, (struct edge_t) {1, 11}, dir1);
	graph__set_edge(b.graph, (struct edge_t) {2, 12}, dir2);
	graph__set_edge(b.graph, (struct edge_t) {3, 13}, dir3);
	graph__set_edge(b.graph, (struct edge_t) {4, 14}, dir4);

	dir1 = graph__get_edge(b.graph, (struct edge_t) {11, 12});
	dir2 = graph__get_edge(b.graph, (struct edge_t) {21, 22});
	dir3 = graph__get_edge(b.graph, (struct edge_t) {31, 32});
	dir4 = graph__get_edge(b.graph, (struct edge_t) {41, 42});
	graph__set_edge(b.graph, (struct edge_t) {11, 12}, NONE);
	graph__set_edge(b.graph, (struct edge_t) {21, 22}, NONE);
	graph__set_edge(b.graph, (struct edge_t) {31, 32}, NONE);
	graph__set_edge(b.graph, (struct edge_t) {41, 42}, NONE);
	struct edge_t edges5[8][2] = {{{21, 31}, {22, 32}},
								  {{21, 31}, {32, 22}},
								  {{31, 21}, {32, 22}},
								  {{31, 21}, {22, 32}},
								  {{22, 32}, {21, 31}},
								  {{22, 32}, {31, 21}},
								  {{32, 22}, {31, 21}},
								  {{32, 22}, {21, 31}}};
	for (int i = 0; i < 8; i++) {
		move.e[0] = edges5[i][0];
		move.e[1] = edges5[i][1];
		if (!board__check_move(&b, &move)) {
			printf("   Wall between walls not possible 2 %zu %zu %zu %zu.\n",
				   edges5[i][0].fr,
				   edges5[i][0].to,
				   edges5[i][1].fr,
				   edges5[i][1].to);
			is_valid = 0;
		}
	}
	move.e[0] = (struct edge_t) {11, 21};
	move.e[1] = (struct edge_t) {12, 22};
	if (board__check_move(&b, &move))
		printf("   Cross wall on wall possible 2\n");
	graph__set_edge(b.graph, (struct edge_t) {1, 11}, dir1);
	graph__set_edge(b.graph, (struct edge_t) {2, 12}, dir2);
	graph__set_edge(b.graph, (struct edge_t) {3, 13}, dir3);
	graph__set_edge(b.graph, (struct edge_t) {4, 14}, dir4);

	graph__set_edge(b.graph, (struct edge_t) {45, 35}, NONE);
	graph__set_edge(b.graph, (struct edge_t) {45, 55}, NONE);
	graph__set_edge(b.graph, (struct edge_t) {45, 44}, NONE);
	move.e[0] = (struct edge_t) {45, 46};
	move.e[0] = (struct edge_t) {55, 56};
	b.pos[0] = 45;
	if (board__check_move(&b, &move)) {
		printf("   Can place a wall blocking movement.\n");
		is_valid = 0;
	}

	board__release(&b);

	return is_valid;
}

int test_board__check_move_no_type()
{
	struct board_t b = init_board();
	struct move_t move = {rand(), {{rand(), rand()}, {rand(), rand()}}, NO_TYPE, rand() % 3};
	int is_valid = board__check_move(&b, &move);

	board__release(&b);
	return is_valid;
}

int test_board__init()
{
	int is_valid = 0;
	struct board_t b;
	struct graph_t* g = graph__square(9);
	struct graph_t* g_beg = graph__square(9);
	struct board_t b_check = {g, g_beg, {10, 10}, {SIZE_MAX, SIZE_MAX}};

	board__init(&b, g);
	is_valid = board__are_equal(&b, &b_check);

	graph__free(b.graph_beginning);
	board__release(&b_check);
	return is_valid;
}

int test_board__are_equal()
{
	int is_valid = 1;

	struct board_t b = init_board();
	struct board_t b_check = init_board();

	if (!board__are_equal(&b, &b_check)) {
		printf("   Board aren't equal\n");
		is_valid = 0;
	}

	b.pos[BLACK] = 1;
	if (board__are_equal(&b, &b_check)) {
		printf("   Change on BLACK's position not detected\n");
		is_valid = 0;
	}
	b.pos[BLACK] = b_check.pos[BLACK];

	b.pos[WHITE] = 1;
	if (board__are_equal(&b, &b_check)) {
		printf("   Change on WHITE's position not detected\n");
		is_valid = 0;
	}
	b.pos[WHITE] = b_check.pos[WHITE];

	b.num_walls[BLACK] = 1;
	if (board__are_equal(&b, &b_check)) {
		printf("   Change on BLACK's number of walls not detected\n");
		is_valid = 0;
	}
	b.num_walls[BLACK] = b_check.num_walls[BLACK];

	b.num_walls[WHITE] = 1;
	if (board__are_equal(&b, &b_check)) {
		printf("   Change on WHITE's number of walls not detected\n");
		is_valid = 0;
	}
	b.num_walls[WHITE] = b_check.num_walls[WHITE];

	graph__set_edge(b.graph, (struct edge_t) {10, 40}, NORTH);
	if (board__are_equal(&b, &b_check)) {
		printf("   Change on graph not detected\n");
		is_valid = 0;
	}
	graph__set_edge(b.graph, (struct edge_t) {10, 40}, NONE);

	graph__set_edge(b.graph_beginning, (struct edge_t) {10, 40}, NORTH);
	if (board__are_equal(&b, &b_check)) {
		printf("   Change on graph_beginning not detected\n");
		is_valid = 0;
	}
	graph__set_edge(b.graph_beginning, (struct edge_t) {10, 40}, NONE);

	graph__claim_node(b.graph, 35, WHITE);
	if (board__are_equal(&b, &b_check)) {
		printf("   Change on graph not detected (starting position)\n");
		is_valid = 0;
	}
	graph__unclaim_node(b.graph, 35, WHITE);

	graph__claim_node(b.graph_beginning, 35, WHITE);
	if (board__are_equal(&b, &b_check)) {
		printf("   Change on graph_beginning not detected (starting position)\n");
		is_valid = 0;
	}
	graph__unclaim_node(b.graph_beginning, 35, WHITE);

	board__release(&b);
	board__release(&b_check);
	return is_valid;
}
