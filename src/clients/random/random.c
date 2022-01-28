#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "client_common.h"
#include "player.h"
#include "move.h"
#include "graph.h"
#include "board.h"

const char* name = "random2";

void local_initialize()
{
}

void local_finalize()
{
}

int add_neighbour_if_valid(enum dir_t dir, struct edge_t valid_edges[], int nb_edges, size_t i, size_t j)
{
	size_t i_neighbour = graph__get_neighbour(b.graph, i, dir);
	size_t j_neighbour = graph__get_neighbour(b.graph, j, dir);
	if (i_neighbour != SIZE_MAX && j_neighbour != SIZE_MAX) {
		struct move_t m = {-1, {{i, j}, {i_neighbour, j_neighbour}}, WALL, client_id};
		if (board__check_move(&b, &m)) {
			valid_edges[nb_edges] = (struct edge_t) {i_neighbour, j_neighbour};
			nb_edges++;
		}
	}

	return nb_edges;
}

/**
 * \brief Check if an edge is available close to the edge (i,j) to place a wall
 *
 * \param i the first node of the origin edge
 * \param j the second node of the origin edge
 * \param dir the direction of the origin edge
 *
 * \return an edge if there is one or no_edge() otherwise
 */
struct edge_t get_valid_place_for_wall(size_t i, size_t j, enum dir_t dir)
{
	// vertexes are not linked
	if (dir < NORTH || dir > EAST)
		return no_edge();

	struct edge_t valid_edges[2];
	int nb_edges = 0;

	if (dir == NORTH || dir == SOUTH) {
		nb_edges = add_neighbour_if_valid(WEST, valid_edges, nb_edges, i, j);
		nb_edges = add_neighbour_if_valid(EAST, valid_edges, nb_edges, i, j);

	} else if (dir == WEST || dir == EAST) {
		nb_edges = add_neighbour_if_valid(NORTH, valid_edges, nb_edges, i, j);
		nb_edges = add_neighbour_if_valid(SOUTH, valid_edges, nb_edges, i, j);
	}

	if (nb_edges == 0) {
		return no_edge();
	}

	int r = rand() % nb_edges;
	return valid_edges[r];
}

/**
 * \brief Shuffle an array
 *
 * \param tab the array
 * \param size the array size
 */
void shuffle(size_t* tab, int size)
{
	for (int i = 0; i < size - 1; i++) {
		int r = rand() % (size - i - 1) + i + 1;

		size_t tmp = tab[i];
		tab[i] = tab[r];
		tab[r] = tmp;
	}
}

struct move_t get_move_move();
struct move_t get_wall_move();

/**
 * \brief Generate a random move of type MOVE
 *
 * \return a move of type MOVE if one is available, a move of type WALL otherwise
 */
struct move_t get_move_move()
{
	size_t possible_moves[6];
	possible_displacements(&b, b.pos[client_id], client_id, possible_moves);
	int i = 0;
	while (possible_moves[i] != SIZE_MAX)
		i++;

	if (i == 0) {
		return get_wall_move();
	}
	size_t pos = possible_moves[rand() % i];
	return (struct move_t) {pos, {no_edge(), no_edge()}, MOVE, client_id};
}

/**
 * \brief Generate a random move of type WALL
 *
 * \return a move of type WALL if one is available, a move of type MOVE otherwise
 */
struct move_t get_wall_move()
{
	int size = b.graph->t->nz;

	size_t ind_tab[size];

	for (int i = 0; i < size; i++) {
		ind_tab[i] = i;
	}

	shuffle(ind_tab, size);

	int ind_tmp = size - 1;
	size_t ind = ind_tab[ind_tmp];
	enum dir_t dir = b.graph->t->data[ind];
	int i = b.graph->t->i[ind];
	int j = b.graph->t->p[ind];
	struct edge_t e = get_valid_place_for_wall(i, j, dir);

	while ((dir < NORTH || dir > EAST || is_no_edge(e)) && size > 1) {
		size--;
		ind_tmp = size - 1;

		ind = ind_tab[ind_tmp];
		dir = b.graph->t->data[ind];
		i = b.graph->t->i[ind];
		j = b.graph->t->p[ind];

		e = get_valid_place_for_wall(i, j, dir);
	}

	if (is_no_edge(e)) {
		return get_move_move();
	}
	return (struct move_t) {-1, {{i, j}, e}, WALL, client_id};
}

struct move_t local_move(struct move_t* previous_move)
{
	UNUSED(previous_move);
	size_t pos;

	// First action must be a move
	if (b.pos[client_id] == SIZE_MAX) {
		size_t* validpos = (size_t*)malloc(b.graph->num_vertices * sizeof(size_t));
		size_t k = 0;
		for (size_t j = 0; j < b.graph->num_vertices; j++) {
			if (gsl_spmatrix_uint_get(b.graph->o, client_id, j) == 1) {
				validpos[k] = j;
				k++;
			}
		}
		pos = validpos[rand() % k];
		free(validpos);

		return (struct move_t) {pos, {no_edge(), no_edge()}, MOVE, client_id};
	}

	enum movetype_t mt = rand() % NO_TYPE;

	if (b.num_walls[client_id] == 0 || mt == MOVE) {
		return get_move_move();

		// WALL
	} else {
		return get_wall_move();
	}
}
