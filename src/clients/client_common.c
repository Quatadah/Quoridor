#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "client_common.h"
#include "player.h"
#include "move.h"
#include "graph.h"
#include "board.h"
#include "movef.h"

struct board_t b;
enum color_t client_id, other_id;

const char* get_player_name()
{
	return name;
}

void initialize(enum color_t id, struct graph_t* graph, size_t num_walls)
{
	client_id = id;
	other_id = ((id == BLACK) ? WHITE : BLACK);

	b.graph = graph;
	b.graph_beginning = graph__copy(graph);
	b.num_walls[0] = num_walls;
	b.num_walls[1] = num_walls;
	b.pos[0] = SIZE_MAX;
	b.pos[1] = SIZE_MAX;

	local_initialize();
}

struct move_t play(struct move_t previous_move)
{
	if (!board__apply_move(&b, &previous_move)) {
		printf("Bad previous move detected in client %s : \n", name);
		print_move(&previous_move, "Other", 0);
		printf("\n");
	}
	struct move_t move = local_move(&previous_move);
	if (!board__apply_move(&b, &move)) {
		printf("Bad move coming from client %s. ", name);
		print_move(&move, name, 0);
		printf("\n");
	}
	return move;
}

void finalize()
{
	local_finalize();

	board__release(&b);
}

void possible_displacements(struct board_t* bo, size_t starting_pos, int id, size_t* displace)
{
	size_t other_id = (id == BLACK) ? WHITE : BLACK;
	size_t neighbours[4];
	graph__get_all_neighbours(bo->graph, starting_pos, neighbours);
	size_t k = 0;

	for (size_t i = 0; i < 4; i++) {
		if (neighbours[i] == bo->pos[id])
			continue;

		if (neighbours[i] == SIZE_MAX)
			displace[k] = SIZE_MAX;

		if (bo->pos[other_id] != SIZE_MAX && neighbours[i] == bo->pos[other_id]) {
			enum dir_t direction = graph__get_edge(bo->graph, (struct edge_t) {bo->pos[id], bo->pos[other_id]});
			size_t next = graph__get_neighbour(bo->graph, bo->pos[other_id], direction);
			if (next == SIZE_MAX) {
				enum dir_t d1 = (direction == NORTH || direction == SOUTH) ? EAST : NORTH;
				enum dir_t d2 = (direction == NORTH || direction == SOUTH) ? WEST : SOUTH;

				size_t nb = graph__get_neighbour(bo->graph, bo->pos[other_id], d1);
				if (nb != SIZE_MAX && nb != bo->pos[id]) {
					displace[k] = nb;
					k++;
				}
				nb = graph__get_neighbour(bo->graph, bo->pos[other_id], d2);
				if (nb != SIZE_MAX && nb != bo->pos[id]) {
					displace[k] = nb;
					k++;
				}
			} else {
				if (next != bo->pos[id]) {
					displace[k] = next;
					k++;
				}
			}
		} else {
			displace[k] = neighbours[i];
			k++;
		}
	}

	displace[k] = SIZE_MAX;
}

struct distdata calc_distance(struct board_t* bo, enum color_t id)
{
	enum color_t other_id = (id == BLACK) ? WHITE : BLACK;
	size_t* file = (size_t*)malloc(bo->graph->num_vertices * sizeof(size_t));
	int start = 0;
	int end = 0;
	size_t* parents = (size_t*)malloc(bo->graph->num_vertices * sizeof(size_t));
	size_t* distances = (size_t*)malloc(bo->graph->num_vertices * sizeof(size_t));

	for (size_t i = 0; i < bo->graph->num_vertices; i++) {
		parents[i] = SIZE_MAX;
	}

	parents[bo->pos[id]] = bo->pos[id];
	file[0] = bo->pos[id];
	end++;
	distances[bo->pos[id]] = 0;
	size_t neighbours[6];
	while (end > start) {
		size_t vertex = file[start];
		if (distances[vertex] == 0)
			possible_displacements(&b, vertex, id, neighbours);
		else
			graph__get_all_neighbours(b.graph, vertex, neighbours);
		for (size_t j = 0; j < ((distances[vertex] == 0) ? 5 : 4); j++) {
			if (neighbours[j] == SIZE_MAX)
				break;
			size_t n = neighbours[j];

			if (parents[n] == SIZE_MAX) {
				parents[n] = vertex;
				file[end] = n;
				end++;
				distances[n] = distances[vertex] + 1;
				if (graph__is_start(bo->graph, n, other_id)) {
					size_t d = distances[n];
					while (parents[n] != bo->pos[id]) {
						n = parents[n];
					}
					free(file);
					free(distances);
					free(parents);
					return (struct distdata) {d, n, 1};
				}
			}
		}
		start++;
	}
	free(file);
	free(distances);
	free(parents);
	return (struct distdata) {SIZE_MAX, SIZE_MAX, 0};
}
