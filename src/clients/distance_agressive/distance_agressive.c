
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "client_common.h"
#include "player.h"
#include "move.h"
#include "graph.h"
#include "board.h"

#define UNUSED(x) (void)(x)

const char* name = "Distance_agg";

void local_initialize()
{
}

void local_finalize()
{
}

struct move_t local_move(struct move_t* previous_move)
{
	UNUSED(previous_move);
	size_t n_id = (client_id == BLACK) ? WHITE : BLACK;
	if (b.pos[client_id] == SIZE_MAX) {
		size_t* validpos = graph__starting_positions(b.graph, client_id);
		size_t* currentpos = validpos;
		int end_pile = 1;
		size_t distance = INT_MAX;
		size_t neighbours[6];
		while (*currentpos != SIZE_MAX) {
			b.pos[client_id] = *currentpos;
			graph__get_all_neighbours(b.graph, *currentpos, neighbours);

			size_t dist = calc_distance(&b, client_id).dist + (neighbours[2] == SIZE_MAX);
			if (dist < distance) {
				end_pile = 1;
				validpos[0] = *currentpos;
				distance = dist;
			} else if (dist == distance) {
				validpos[end_pile] = *currentpos;
				end_pile++;
			}
			currentpos++;
		}
		b.pos[client_id] = SIZE_MAX;
		size_t pos = validpos[rand() % end_pile];
		free(validpos);
		return (struct move_t) {pos, {no_edge(), no_edge()}, MOVE, client_id};
	} else {
		struct distdata distdata1 = calc_distance(&b, client_id);
		struct distdata distdata2 = calc_distance(&b, n_id);
		if (b.num_walls[client_id] > 0 && distdata1.dist > distdata2.dist) {
			size_t save_pos = b.pos[client_id];
			b.pos[client_id] = distdata1.next;
			int diff_distance_move = calc_distance(&b, n_id).dist - calc_distance(&b, client_id).dist;
			b.pos[client_id] = save_pos;

			struct edge_t edge_to_cut = {b.pos[n_id], distdata2.next};
			enum dir_t dir = graph__get_edge(b.graph, edge_to_cut);
			enum dir_t d1 = (dir == NORTH || dir == SOUTH) ? EAST : NORTH;
			enum dir_t d2 = (dir == NORTH || dir == SOUTH) ? WEST : SOUTH;
			size_t nb1 = graph__get_neighbour(b.graph_beginning, b.pos[n_id], d1);
			size_t nb2 = graph__get_neighbour(b.graph_beginning, b.pos[n_id], d2);
			size_t nbb1 = (nb1 == SIZE_MAX) ? SIZE_MAX : graph__get_neighbour(b.graph, nb1, dir);
			size_t nbb2 = (nb2 == SIZE_MAX) ? SIZE_MAX : graph__get_neighbour(b.graph, nb2, dir);

			if (nb1 != SIZE_MAX && nbb1 != SIZE_MAX) {
				struct edge_t edge_candidate_1 = {nb1, nbb1};
				if (nb2 != SIZE_MAX && nbb2 != SIZE_MAX) {
					struct edge_t edge_candidate_2 = {nb2, nbb2};

					enum dir_t save_e = graph__get_edge(b.graph, edge_candidate_1);
					graph__set_edge(b.graph, edge_to_cut, NONE);

					graph__set_edge(b.graph, edge_candidate_1, NONE);
					struct distdata distdata_client = calc_distance(&b, client_id);
					struct distdata distdata_other = calc_distance(&b, n_id);
					int diff_distance1 = distdata_other.dist - distdata_client.dist;
					graph__set_edge(b.graph, edge_candidate_1, save_e);

					save_e = graph__get_edge(b.graph, edge_candidate_2);
					graph__set_edge(b.graph, edge_candidate_2, NONE);
					distdata_client = calc_distance(&b, client_id);
					distdata_other = calc_distance(&b, n_id);
					int diff_distance2 = distdata_other.dist - distdata_client.dist;
					graph__set_edge(b.graph, edge_candidate_2, save_e);

					graph__set_edge(b.graph, edge_to_cut, dir);

					struct move_t move1 = (struct move_t) {0, {edge_to_cut, edge_candidate_1}, WALL, client_id};
					struct move_t move2 = (struct move_t) {0, {edge_to_cut, edge_candidate_2}, WALL, client_id};

					int choice1_is_possible = board__check_move(&b, &move1);
					int choice2_is_possible = board__check_move(&b, &move2);

					if ((!choice1_is_possible || diff_distance_move > diff_distance1)
						&& (!choice2_is_possible || diff_distance_move > diff_distance2))
						return (struct move_t) {distdata1.next, {no_edge(), no_edge()}, MOVE, client_id};

					if (choice1_is_possible && diff_distance1 > diff_distance2)
						return move1;
					else if (choice2_is_possible && (diff_distance1 < diff_distance2 || rand() % 2 == 0))
						return move2;
					else if (choice1_is_possible)
						return move1;
				} else {
					struct move_t possible_move = (struct move_t) {0, {edge_to_cut, edge_candidate_1}, WALL, client_id};
					if (board__check_move(&b, &possible_move))
						return possible_move;
				}
			} else {
				if (nb2 != SIZE_MAX && nbb2 != SIZE_MAX) {
					struct move_t possible_move = (struct move_t) {0, {edge_to_cut, {nb2, nbb2}}, WALL, client_id};
					if (board__check_move(&b, &possible_move))
						return possible_move;
				}
			}
		}
		return (struct move_t) {distdata1.next, {no_edge(), no_edge()}, MOVE, client_id};
	}
}
