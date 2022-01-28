#include "client_common.h"
#include "math.h"
#include "graph_optimized.h"
#include "time.h"

#define NB_ITERATIONS 4000

const char* name = "MonteCarlo";

struct mc_node* memory;
size_t nb_node_allocated;
size_t mc_node_children_size;

struct board_optimized bo;

struct mc_node {
	struct move_t move; // NO_TYPE for root
	double b, n, calc_sum;
	unsigned int nb_children_taken, nb_children_free;
	struct mc_node* children; // NULL at the beginning
};

double mc_simulate(enum color_t id)
{
	enum color_t n_id = id == BLACK ? WHITE : BLACK;
	if (bo.pos[BLACK] == SIZE_MAX || bo.pos[WHITE] == SIZE_MAX)
		return 0.5;
	if (board_optimized__is_start(&bo, bo.pos[n_id], id))
		return 0.0;
	if (board_optimized__is_start(&bo, bo.pos[id], n_id))
		return 1.0;
	return 0.5
		   * (1.0
			  + tanh(((double)board_optimized__calc_distance(&bo, n_id).dist - (double)board_optimized__calc_distance(&bo, id).dist)
					 / 20.0));
}

struct mc_node mc_node__create(struct move_t move)
{
	return (struct mc_node) {move, 0.0, 0.0, 0.0, 0, UINT_MAX, NULL};
}

void mc_node__populate_children(struct mc_node* node)
{
	if ((bo.pos[other_id] != SIZE_MAX && board_optimized__is_start(&bo, bo.pos[other_id], client_id))
		|| (bo.pos[client_id] != SIZE_MAX && board_optimized__is_start(&bo, bo.pos[client_id], other_id)))
		return;

	/*node->children = memory + nb_node_allocated * mc_node_children_size;
	nb_node_allocated++;*/
	node->children = malloc(sizeof(struct mc_node) * mc_node_children_size);
	unsigned int nb_children = 0;
	enum color_t id = node->move.c == BLACK ? WHITE : BLACK;
	struct move_t move = {0, {no_edge(), no_edge()}, MOVE, id};
	if (bo.pos[id] == SIZE_MAX) {
		size_t* positions = bo.starting_positions[id];
		size_t* current_pos = positions;
		while (*current_pos != SIZE_MAX) {
			move.m = *current_pos;
			node->children[nb_children] = mc_node__create(move);
			nb_children++;
			current_pos++;
		}
	} else {
		size_t positions[6];
		board_optimized__possible_displacements(&bo, bo.pos[id], id, positions);
		size_t* current_pos = positions;
		while (*current_pos != SIZE_MAX) {
			move.m = *current_pos;
			node->children[nb_children] = mc_node__create(move);
			nb_children++;
			current_pos++;
		}

		move.t = WALL;
		struct edge_t(*current_wall)[2] = bo.walls;
		while (!is_no_edge(*current_wall[0]) && !is_no_edge(*current_wall[1])) {
			move.e[0].fr = (*current_wall)[0].fr;
			move.e[0].to = (*current_wall)[0].to;
			move.e[1].fr = (*current_wall)[1].fr;
			move.e[1].to = (*current_wall)[1].to;
			if (board_optimized__check_move(&bo, &move)) {
				node->children[nb_children] = mc_node__create(move);
				nb_children++;
			}
			current_wall++;
		}
	}

	node->nb_children_free = nb_children;
	node->children = realloc(node->children, sizeof(struct mc_node) * nb_children);
}

double mc_node__expand_one_child(struct mc_node* node)
{
	if (node->nb_children_taken == 0) {
		mc_node__populate_children(&node[node->nb_children_taken]);
		if (node->nb_children_free == UINT_MAX)
			return mc_simulate(node->move.c);
	}

	int index = node->nb_children_taken + rand() % node->nb_children_free;
	struct move_t temp = node->children[index].move;
	node->children[index].move = node->children[node->nb_children_taken].move;
	node->children[node->nb_children_taken].move = temp;

	struct save_move_t save = board_optimized__apply_move_with_save(&bo, &node->children[node->nb_children_taken].move);

	node->children[node->nb_children_taken].b = mc_simulate(node->children[node->nb_children_taken].move.c);
	node->children[node->nb_children_taken].n = 1.0;
	node->children[node->nb_children_taken].calc_sum = 1.0;

	board_optimized__apply_save(&bo, &save);

	node->nb_children_free--;
	node->nb_children_taken++;

	return (1.0 - node->children[node->nb_children_taken - 1].b);
}

double mc_node__select(struct mc_node* node)
{
	/* 1. Selection */
	struct save_move_t save = board_optimized__apply_move_with_save(&bo, &node->move);
	double T_calc = sqrt(2 * log(node->n));
	double simulation_result = 0.0;
	if (node->nb_children_free == 0) {
		double value = 0.0;
		for (unsigned int i = 0; i < node->nb_children_taken; i++)
			value += node->children[i].b / node->children[i].n + T_calc / sqrt(node->nb_children_taken);

		value *= ((double)rand() / (double)RAND_MAX);
		for (unsigned int i = 0; i < node->nb_children_taken; i++) {
			double value_child = node->children[i].b / node->children[i].n + T_calc / sqrt(node->nb_children_taken);
			if (value < value_child) {
				simulation_result = mc_node__select(&node->children[i]);
				/* 4. Backpropagation */
				node->n += 1.0;
				node->b += simulation_result;
				node->calc_sum -= 1.0 / sqrt(node->children[i].n - 1);
				node->calc_sum += 1.0 / sqrt(node->children[i].n);
				break;
			} else
				value -= value_child;
		}
	} else {
		/* 2. Expansion (returns 3. Simulation) */
		simulation_result = mc_node__expand_one_child(node);
		node->n += 1.0;
		node->b += simulation_result;
		node->calc_sum += 1.0;
	}
	board_optimized__apply_save(&bo, &save);
	return (1.0 - simulation_result);
}

void mc_node__release(struct mc_node* node)
{
	for (unsigned int k = 0; k < node->nb_children_taken; k++) {
		mc_node__release(&node->children[k]);
	}
	free(node->children);
}

struct mc_node mc_root;

void mc_change_mc_root(unsigned int i)
{
	struct mc_node* temp = mc_root.children;
	for (unsigned int k = 0; k < mc_root.nb_children_taken; k++) {
		if (k == i)
			continue;
		mc_node__release(&mc_root.children[k]);
	}
	mc_root = mc_root.children[i];
	free(temp);

	mc_root.move.t = NO_TYPE;
}

void local_initialize()
{
	mc_node_children_size = ((b.graph->num_vertices * 2) * (b.graph->num_vertices * 2 - 1) / 2 + 5);
	// memory = malloc(NB_ITERATIONS * sizeof(struct mc_node) * mc_node_children_size);
	// printf("%p\n", memory);

	board_optimized__initialize(b.graph, &bo);

	mc_root = mc_node__create((struct move_t) {0, {no_edge(), no_edge()}, NO_TYPE, other_id});
}

void local_finalize()
{
	// free(memory);

	board_optimized__release(&bo);
	mc_node__release(&mc_root);
}

int move__are_equal(struct move_t* m1, struct move_t* m2)
{
	if (m1->c != m2->c || m1->t != m2->t)
		return 0;

	switch (m1->t) {
		case MOVE:
			return m1->m == m2->m;

		case WALL:
			if (m1->e[0].to < m1->e[0].fr) {
				size_t temp = m1->e[0].fr;
				m1->e[0].fr = m1->e[0].to;
				m1->e[0].to = temp;
			}
			if (m1->e[1].to < m1->e[1].fr) {
				size_t temp = m1->e[1].fr;
				m1->e[1].fr = m1->e[1].to;
				m1->e[1].to = temp;
			}
			if (m2->e[0].to < m2->e[0].fr) {
				size_t temp = m2->e[0].fr;
				m2->e[0].fr = m2->e[0].to;
				m2->e[0].to = temp;
			}
			if (m2->e[1].to < m2->e[1].fr) {
				size_t temp = m2->e[1].fr;
				m2->e[1].fr = m2->e[1].to;
				m2->e[1].to = temp;
			}

			if (m1->e[0].fr > m1->e[1].to) {
				struct edge_t temp = m1->e[1];
				m1->e[1] = m1->e[0];
				m1->e[0] = temp;
			}
			if (m2->e[0].fr > m2->e[1].to) {
				struct edge_t temp = m2->e[1];
				m2->e[1] = m2->e[0];
				m2->e[0] = temp;
			}

			return m1->e[0].fr == m2->e[0].fr && m1->e[1].fr == m2->e[1].fr && m1->e[0].to == m2->e[0].to && m1->e[1].to == m2->e[1].to;

		case NO_TYPE:
			return 1;

		default:
			return -1;
	}
}

struct move_t local_move(struct move_t* previous_move)
{
	for (unsigned int k = 0; k < mc_root.nb_children_taken; k++) {
		if (move__are_equal(previous_move, &mc_root.children[k].move)) {
			mc_change_mc_root(k);
			goto children_found;
		}
	}
	mc_node__release(&mc_root);
	mc_root = mc_node__create((struct move_t) {0, {no_edge(), no_edge()}, NO_TYPE, other_id});

children_found:
	board_optimized__apply_move_with_save(&bo, previous_move);

	nb_node_allocated = 0;
	// mc_root = mc_node__create((struct move_t) {0, {no_edge(), no_edge()}, NO_TYPE, other_id});

	/*for (int i = 0; i < NB_ITERATIONS; i++) {
		mc_node__select(&mc_root);
	}*/

	clock_t start = clock();
	int nb = 0;
	do {
		mc_node__select(&mc_root);
		nb++;
	} while ((clock() - start) * 2 / CLOCKS_PER_SEC < 1 && nb < NB_ITERATIONS);

	struct move_t move = {0, {no_edge(), no_edge()}, NO_TYPE, client_id};
	double b_max = -DBL_MAX;
	unsigned int index = UINT_MAX;
	for (unsigned int k = 0; k < mc_root.nb_children_taken; k++) {
		double val = mc_root.children[k].b / mc_root.children[k].n;
		if (val > b_max) {
			b_max = val;
			move = mc_root.children[k].move;
			index = k;
		}
	}
	// printf("%ff\n", b_max);

	board_optimized__apply_move_with_save(&bo, &move);

	mc_change_mc_root(index);

	return move;
}
