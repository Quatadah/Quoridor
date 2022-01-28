#include "graphf.h"

struct graph_t* graph__init(size_t num_vertices)
{
	struct graph_t* graph = (struct graph_t*)malloc(sizeof(struct graph_t));
	graph->num_vertices = num_vertices;
	graph->t = gsl_spmatrix_uint_alloc(num_vertices, num_vertices);
	graph->o = gsl_spmatrix_uint_alloc(2, num_vertices);
	return graph;
}

struct graph_t* graph__copy(struct graph_t* g)
{
	struct graph_t* graph = (struct graph_t*)malloc(sizeof(struct graph_t));
	gsl_spmatrix_uint* t = gsl_spmatrix_uint_alloc(g->num_vertices, g->num_vertices);
	gsl_spmatrix_uint_memcpy(t, g->t);
	gsl_spmatrix_uint* o = gsl_spmatrix_uint_alloc(2, g->num_vertices);
	gsl_spmatrix_uint_memcpy(o, g->o);
	graph->num_vertices = g->num_vertices;
	graph->t = t;
	graph->o = o;
	return graph;
}

void graph__claim_node(struct graph_t* g, size_t s, enum color_t id)
{
	gsl_spmatrix_uint_set(g->o, id, s, 1);
}

void graph__unclaim_node(struct graph_t* g, size_t s, enum color_t id)
{
	gsl_spmatrix_uint_set(g->o, id, s, 0);
}

unsigned int graph__is_start(struct graph_t* g, size_t s, enum color_t id)
{
	return gsl_spmatrix_uint_get(g->o, id, s);
}

void graph__set_edge(struct graph_t* g, struct edge_t e, enum dir_t d)
{
	gsl_spmatrix_uint_set(g->t, e.fr, e.to, d);
	if (d == 0) {
		gsl_spmatrix_uint_set(g->t, e.to, e.fr, d);
	} else if (d > 0) {
		gsl_spmatrix_uint_set(g->t, e.to, e.fr, d == 1 ? 2 : d == 2 ? 1 : d == 3 ? 4 : 3);
	}
}

enum dir_t graph__get_edge(struct graph_t* g, struct edge_t e)
{
	return gsl_spmatrix_uint_get(g->t, e.fr, e.to);
}

void graph__free(struct graph_t* g)
{
	gsl_spmatrix_uint_free(g->t);

	gsl_spmatrix_uint_free(g->o);

	free(g);
}

struct graph_t* graph__square(size_t n)
{
	struct graph_t* g = graph__init(n * n);
	for (size_t i = 0; i < n; i++) {
		for (size_t j = 0; j < n; j++) {
			size_t nd = i * n + j;
			if (j < n - 1)
				graph__set_edge(g, (struct edge_t) {nd, nd + 1}, EAST);
			if (i < n - 1)
				graph__set_edge(g, (struct edge_t) {nd, nd + n}, SOUTH);
		}
		graph__claim_node(g, i, BLACK);
		graph__claim_node(g, n * (n - 1) + i, WHITE);
	}
	return g;
}

struct graph_t* graph__toric(size_t n)
{
	size_t param = n / 3;
	struct graph_t* g = graph__init(n * n - param * param);
	for (size_t i = 0; i < param; i++) {
		for (size_t j = 0; j < param; j++) {
			size_t nd = (n + i) * param + j;
			if (j < param - 1)
				graph__set_edge(g, (struct edge_t) {nd, nd + 1}, EAST);
			if (i == 0)
				graph__set_edge(g, (struct edge_t) {nd, nd - n}, NORTH);
			if (i < param - 1)
				graph__set_edge(g, (struct edge_t) {nd, nd + param}, SOUTH);
			else
				graph__set_edge(g, (struct edge_t) {nd, nd + (param + 1) * param}, SOUTH);

			nd = (n + i + param) * param + j;
			if (j < param - 1)
				graph__set_edge(g, (struct edge_t) {nd, nd + 1}, EAST);
			if (i == 0)
				graph__set_edge(g, (struct edge_t) {nd, nd - (param + 1) * param}, NORTH);
			if (i < param - 1)
				graph__set_edge(g, (struct edge_t) {nd, nd + param}, SOUTH);
			else
				graph__set_edge(g, (struct edge_t) {nd, nd + 3 * param}, SOUTH);
		}

		for (size_t j = 0; j < n; j++) {
			size_t nd = i * n + j;
			if (j < n - 1)
				graph__set_edge(g, (struct edge_t) {nd, nd + 1}, EAST);
			if (i < param - 1)
				graph__set_edge(g, (struct edge_t) {nd, nd + n}, SOUTH);

			nd = (i + param) * n + j + 2 * param * param;
			if (j < n - 1)
				graph__set_edge(g, (struct edge_t) {nd, nd + 1}, EAST);
			if (i < param - 1)
				graph__set_edge(g, (struct edge_t) {nd, nd + n}, SOUTH);
		}
	}

	for (size_t i = 0; i < n; i++) {
		graph__claim_node(g, i, BLACK);
		graph__claim_node(g, (2 * param - 1) * n + i + 2 * param * param, WHITE);
	}
	return g;
}

struct graph_t* graph__s_shaped(size_t n)
{
	size_t param = n / 5;
	struct graph_t* g = graph__init(n * n - 3 * 4 * param * param);
	for (size_t i = 0; i < param; i++) {
		for (size_t j = 0; j < param; j++) {
			size_t nd = i * param + j;
			if (j < param - 1)
				graph__set_edge(g, (struct edge_t) {nd, nd + 1}, EAST);
			graph__set_edge(g, (struct edge_t) {nd, nd + param}, SOUTH);

			nd = (i + param + n) * param + j;
			if (j < param - 1)
				graph__set_edge(g, (struct edge_t) {nd, nd + 1}, EAST);
			if (i < param - 1)
				graph__set_edge(g, (struct edge_t) {nd, nd + param}, SOUTH);
			else
				graph__set_edge(g, (struct edge_t) {nd, nd + n}, SOUTH);
			if (i == 0)
				graph__set_edge(g, (struct edge_t) {nd, nd - param}, NORTH);

			nd = (i + 2 * (param + n)) * param + j;
			if (j < param - 1)
				graph__set_edge(g, (struct edge_t) {nd, nd + 1}, EAST);
			if (i == 0)
				graph__set_edge(g, (struct edge_t) {nd, nd - n}, NORTH);
			else
				graph__set_edge(g, (struct edge_t) {nd, nd - param}, NORTH);
		}

		for (size_t j = 0; j < n; j++) {
			size_t nd = i * n + j + param * param;
			if (j < n - 1)
				graph__set_edge(g, (struct edge_t) {nd, nd + 1}, EAST);
			if (i < param - 1)
				graph__set_edge(g, (struct edge_t) {nd, nd + n}, SOUTH);

			nd = (i + param) * n + j + 2 * param * param;
			if (j < n - 1)
				graph__set_edge(g, (struct edge_t) {nd, nd + 1}, EAST);
			if (i < param - 1)
				graph__set_edge(g, (struct edge_t) {nd, nd + n}, SOUTH);
		}
	}

	for (size_t i = 0; i < param; i++) {
		graph__claim_node(g, i, BLACK);
		graph__claim_node(g, param * (3 * param - 1) + 2 * n * param + i, WHITE);
	}
	return g;
}

struct graph_t* graph__h_shaped(size_t n)
{
	size_t param = n / 3;
	struct graph_t* g = graph__init(n * n - 2 * param * param);
	for (size_t i = 0; i < param; i++) {
		for (size_t j = 0; j < n; j++) {
			size_t nd = j * param + i;
			if (i < param - 1)
				graph__set_edge(g, (struct edge_t) {nd, nd + 1}, EAST);
			if (j < n - 1)
				graph__set_edge(g, (struct edge_t) {nd, nd + param}, SOUTH);

			nd = (j + n + param) * param + i;
			if (i < param - 1)
				graph__set_edge(g, (struct edge_t) {nd, nd + 1}, EAST);
			if (j < n - 1)
				graph__set_edge(g, (struct edge_t) {nd, nd + param}, SOUTH);
		}

		for (size_t j = 0; j < param; j++) {
			size_t nd = (n + i) * param + j;
			if (j == 0)
				graph__set_edge(g, (struct edge_t) {nd, nd - (2 * param - 1) * param - 1}, WEST);
			if (j < param - 1)
				graph__set_edge(g, (struct edge_t) {nd, nd + 1}, EAST);
			else
				graph__set_edge(g, (struct edge_t) {nd, nd + (2 * param - 1) * param + 1}, EAST);

			if (i < param - 1)
				graph__set_edge(g, (struct edge_t) {nd, nd + param}, SOUTH);
		}
		graph__claim_node(g, i, BLACK);
		graph__claim_node(g, (n + param) * param + i, BLACK);
		graph__claim_node(g, (n - 1) * param + i, WHITE);
		graph__claim_node(g, (2 * n + param - 1) * param + i, WHITE);
	}
	return g;
}

size_t graph__get_neighbour(struct graph_t* g, size_t n, enum dir_t dir)
{
	for (size_t j = 0; j < g->num_vertices; j++) {
		if (gsl_spmatrix_uint_get(g->t, n, j) == dir)
			return j;
	}
	return SIZE_MAX;
}

void graph__get_all_neighbours(struct graph_t* g, size_t n, size_t* neighbours)
{
	int k = 0;
	for (size_t j = 0; j < g->num_vertices; j++) {
		if (k >= 4)
			break;
		if (gsl_spmatrix_uint_get(g->t, n, j) > 0) {
			neighbours[k] = j;
			k++;
		}
	}

	for (size_t j = k; j < 4; j++) {
		neighbours[j] = SIZE_MAX;
	}
}

size_t* graph__starting_positions(struct graph_t* g, enum color_t id)
{
	size_t* validpos = (size_t*)malloc((g->num_vertices + 1) * sizeof(size_t));
	size_t k = 0;
	for (size_t j = 0; j < g->num_vertices; j++) {
		if (gsl_spmatrix_uint_get(g->o, id, j) == 1) {
			validpos[k] = j;
			k++;
		}
	}
	validpos[k] = SIZE_MAX;
	k++;
	validpos = (size_t*)realloc(validpos, k * sizeof(size_t));
	return validpos;
}
