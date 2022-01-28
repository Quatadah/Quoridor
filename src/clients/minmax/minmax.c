
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "client_common.h"
#include "player.h"
#include "move.h"
#include "graph.h"
#include "board.h"

#define UNUSED(x) (void)(x)

const char* name = "minmax";

void local_initialize()
{
}

void local_finalize()
{
}

struct minmaxdata {
	long long int eval;
	struct move_t M;
};

int move_equal(struct move_t* M1, struct move_t* M2)
{
	if (M1->t != M2->t)
		return 0;

	switch (M1->t) {
		case WALL:
			if (M1->c != M2->c)
				return 0;

			int flag1 = (M1->e[0].fr == M2->e[0].fr) && (M1->e[0].to == M2->e[0].to) && (M1->e[1].fr == M2->e[1].fr)
						&& (M1->e[1].to == M2->e[1].to);
			int flag2 = (M1->e[0].fr == M2->e[0].to) && (M1->e[0].to == M2->e[0].fr) && (M1->e[1].fr == M2->e[1].to)
						&& (M1->e[1].to == M2->e[1].fr);

			int flag3 = (M1->e[0].fr == M2->e[1].fr) && (M1->e[0].to == M2->e[1].to) && (M1->e[1].fr == M2->e[0].fr)
						&& (M1->e[1].to == M2->e[0].to);
			int flag4 = (M1->e[0].fr == M2->e[1].to) && (M1->e[0].to == M2->e[1].fr) && (M1->e[1].fr == M2->e[0].to)
						&& (M1->e[1].to == M2->e[0].fr);

			return flag1 || flag2 || flag3 || flag4;
		case MOVE:
			return (M1->m == M2->m) && (M1->c == M2->c);
		case NO_TYPE:
			return 1;
		default:
			return -1;
	}
}

struct move_t* walls_around(struct board_t* bo, struct move_t* start_moves, struct move_t* cur_moves, size_t pos, int id)
{
	size_t neighbours[6];
	graph__get_all_neighbours(bo->graph, pos, neighbours);
	for (size_t i = 0; i < 4; i++) {
		size_t n_pos = neighbours[i];
		if (n_pos == SIZE_MAX)
			break;
		enum dir_t e = graph__get_edge(bo->graph, (struct edge_t) {pos, n_pos});
		enum dir_t d1 = (e == NORTH || e == SOUTH) ? EAST : NORTH;
		enum dir_t d2 = (e == NORTH || e == SOUTH) ? WEST : SOUTH;

		size_t p1 = graph__get_neighbour(bo->graph, pos, d1);
		size_t n_p1 = graph__get_neighbour(bo->graph, n_pos, d1);

		if (p1 != SIZE_MAX && n_p1 != SIZE_MAX) {
			struct move_t M = (struct move_t) {0, {(struct edge_t) {pos, n_pos}, (struct edge_t) {p1, n_p1}}, WALL, id};
			struct move_t* strt = start_moves;
			int flag = 1;
			while (strt != cur_moves) {
				if (move_equal(strt, &M)) {
					flag = 0;
					break;
				}
				strt++;
			}

			if (flag && board__check_move(bo, &M)) {
				*cur_moves = M;
				cur_moves++;
			}
		}

		size_t p2 = graph__get_neighbour(bo->graph, pos, d2);
		size_t n_p2 = graph__get_neighbour(bo->graph, n_pos, d2);

		if (p2 != SIZE_MAX && n_p2 != SIZE_MAX) {
			struct move_t M = (struct move_t) {0, {(struct edge_t) {pos, n_pos}, (struct edge_t) {p2, n_p2}}, WALL, id};
			struct move_t* strt = start_moves;
			int flag = 1;
			while (strt != cur_moves) {
				if (move_equal(strt, &M)) {
					flag = 0;
					break;
				}
				strt++;
			}
			if (flag && board__check_move(bo, &M)) {
				*cur_moves = M;
				cur_moves++;
			}
		}
	}
	return cur_moves;
}

struct move_t* possible_moves(struct board_t* bo, int id)
{
	if (bo->pos[id] == SIZE_MAX) {
		size_t* start = graph__starting_positions(bo->graph, id);
		size_t size = 0;
		while (start[size] != SIZE_MAX) {
			size++;
		}
		struct move_t* moves = (struct move_t*)malloc((size + 1) * sizeof(struct move_t));

		for (size_t i = 0; i < size; i++) {
			moves[i] = (struct move_t) {start[i], {no_edge(), no_edge()}, MOVE, id};
		}
		free(start);
		moves[size] = (struct move_t) {0, {no_edge(), no_edge()}, NO_TYPE, 0};
		return moves;
	}

	size_t n_id = (id == BLACK) ? WHITE : BLACK;
	size_t* ps = graph__starting_positions(bo->graph, n_id);
	size_t displacements[6];
	possible_displacements(bo, bo->pos[id], id, displacements);
	size_t size1 = 0;
	while (ps[size1] != SIZE_MAX) {
		size1++;
	}
	size_t size2 = 0;
	while (displacements[size2] != SIZE_MAX) {
		size2++;
	}
	struct move_t* moves = (struct move_t*)malloc((8 * size1 + size2 + 1) * sizeof(struct move_t));
	struct move_t* cur_moves = moves;
	for (size_t k = 0; k < size1; k++) {
		cur_moves = walls_around(bo, moves, cur_moves, ps[k], id);
	}
	free(ps);
	cur_moves = walls_around(bo, moves, cur_moves, bo->pos[n_id], id);

	for (size_t k = 0; k < size2; k++) {
		*cur_moves = (struct move_t) {displacements[k], {no_edge(), no_edge()}, MOVE, id};
		cur_moves++;
	}
	*cur_moves = (struct move_t) {0, {no_edge(), no_edge()}, NO_TYPE, 0};
	return moves;
}

long long int minmax_evaluation(struct board_t* bo)
{
	long long int d1 = (bo->pos[BLACK] == SIZE_MAX) ? 0 : calc_distance(bo, BLACK).dist;
	long long int d2 = (bo->pos[WHITE] == SIZE_MAX) ? 0 : calc_distance(bo, WHITE).dist;
	return d1 - d2;
}

void undo_wall(struct board_t* bo, struct move_t* M, size_t e1, size_t e2)
{
	graph__set_edge(bo->graph, M->e[0], e1);
	graph__set_edge(bo->graph, M->e[1], e2);
	bo->num_walls[M->c]++;
}

void undo_move(struct board_t* bo, struct move_t* M, size_t m)
{
	bo->pos[M->c] = m;
}

struct minmaxdata minmax(struct board_t* bo, int id, int depth, long long int alpha, long long int beta)
{
	if (depth == 0)
		return (struct minmaxdata) {minmax_evaluation(bo), (struct move_t) {0, {no_edge(), no_edge()}, NO_TYPE, 0}};
	struct move_t* moves = possible_moves(bo, id);
	int k = 0;
	struct move_t M = moves[k];
	long long int ev = (id == WHITE) ? LLONG_MIN : LLONG_MAX;
	struct move_t res = (struct move_t) {0, {no_edge(), no_edge()}, NO_TYPE, 0};

	while (M.t != NO_TYPE && beta > alpha) {
		// struct board_t* b_cpy = board__copy(bo);
		// board__apply_move(b_cpy, &M);
		size_t m, e1, e2;
		if (M.t == MOVE) {
			m = bo->pos[id];
		} else {
			e1 = graph__get_edge(bo->graph, M.e[0]);
			e2 = graph__get_edge(bo->graph, M.e[1]);
		}
		board__apply_move(bo, &M);
		long long int ev_c = minmax(bo, (id == BLACK) ? WHITE : BLACK, depth - 1, alpha, beta).eval;
		if (M.t == MOVE) {
			undo_move(bo, &M, m);
		} else {
			undo_wall(bo, &M, e1, e2);
		}
		// board__release(b_cpy);
		// free(b_cpy);
		if (id == BLACK) {
			if (ev_c < ev) {
				ev = ev_c;
				res = M;
			}
			beta = (beta > ev_c) ? ev_c : beta;
		} else {
			if (ev_c > ev) {
				ev = ev_c;
				res = M;
			}
			alpha = (alpha > ev_c) ? alpha : ev_c;
		}
		k++;
		M = moves[k];
	}

	free(moves);
	return (struct minmaxdata) {ev, res};
}

struct move_t local_move(struct move_t* previous_move)
{
	UNUSED(previous_move);
	size_t id = client_id;
	struct minmaxdata A = minmax(&b, id, 2, LLONG_MIN, LLONG_MAX);

	printf("EVAL : %lld\n", A.eval);
	return A.M;
}
