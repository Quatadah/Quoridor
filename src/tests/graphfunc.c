#include <stdio.h>
#include <stdlib.h>
#include "graphf.h"

int test_graph__init()
{
	struct graph_t* g = graph__init(10);
	int is_valid = 0;
	is_valid = g->num_vertices == 10 && g->t->size1 == 10 && g->t->size2 == 10 && g->t->nz == 0 && g->o->size1 == 2 && g->o->size2 == 10
			   && g->t->nz == 0;
	graph__free(g);
	return is_valid;
}

int test_graph__copy()
{
	struct graph_t* g = graph__init(10);
	struct graph_t* gcopy = graph__copy(g);
	int is_valid = (g->num_vertices == gcopy->num_vertices);
	for (int i = 0; i < 10; i++) {
		for (int j = 0; j < 10; j++) {
			is_valid = gsl_spmatrix_uint_get(gcopy->t, i, j) == gsl_spmatrix_uint_get(g->t, i, j);
			if (i < 2)
				is_valid = gsl_spmatrix_uint_get(gcopy->o, i, j) == gsl_spmatrix_uint_get(g->o, i, j);
		}
	}
	graph__free(gcopy);
	graph__free(g);
	return is_valid;
}

int test_graph__set_edge()
{
	int is_valid = 0;
	struct graph_t* g = graph__init(21);
	struct edge_t EastEdge = {15, 16};
	struct edge_t NorthEdge = {10, 20};
	struct edge_t WestEdge = {18, 17};
	struct edge_t SouthEdge = {19, 9};

	graph__set_edge(g, EastEdge, EAST);
	graph__set_edge(g, NorthEdge, NORTH);
	graph__set_edge(g, WestEdge, WEST);
	graph__set_edge(g, SouthEdge, SOUTH);

	is_valid = gsl_spmatrix_uint_get(g->t, 15, 16) == EAST && gsl_spmatrix_uint_get(g->t, 16, 15) == WEST
			   && gsl_spmatrix_uint_get(g->t, 10, 20) == NORTH && gsl_spmatrix_uint_get(g->t, 20, 10) == SOUTH
			   && gsl_spmatrix_uint_get(g->t, 18, 17) == WEST && gsl_spmatrix_uint_get(g->t, 17, 18) == EAST
			   && gsl_spmatrix_uint_get(g->t, 19, 9) == SOUTH && gsl_spmatrix_uint_get(g->t, 9, 19) == NORTH;
	graph__free(g);
	return is_valid;
}

int test_graph__get_edge()
{
	int is_valid = 0;
	struct graph_t* g = graph__init(15);
	struct edge_t edge = {13, 14};
	graph__set_edge(g, edge, EAST);
	int returnedEdge = graph__get_edge(g, edge);
	is_valid = returnedEdge == EAST;
	graph__free(g);
	return is_valid;
}

int test_graph__square()
{
	int is_valid = 1;
	struct graph_t* g = graph__square(10);
	enum dir_t expected;
	for (size_t i = 0; i < 100; i++)
		for (size_t j = 0; j < 100; j++) {
			if (i % 10 != 0 && j == (i - 1)) {
				expected = WEST;
			} else if (i % 10 != 9 && j == (i + 1)) {
				expected = EAST;
			} else if (i > 9 && j == (i - 10)) {
				expected = NORTH;
			} else if (i < 90 && j == (i + 10)) {
				expected = SOUTH;
			} else
				expected = NONE;
			if ((graph__get_edge(g, (struct edge_t) {i, j}) != expected)) {
				printf("   Wrong value for %zu & %zu, got %u, expected %u\n", i, j, graph__get_edge(g, (struct edge_t) {i, j}), expected);
				is_valid = 0;
			}
		}

	for (size_t i = 0; i < 100; i++) {
		if (i < 10) {
			if (!graph__is_start(g, i, BLACK)) {
				printf("   %zu is not starting area of BLACK\n", i);
				is_valid = 0;
			}
		} else {
			if (graph__is_start(g, i, BLACK)) {
				printf("   %zu is starting area of BLACK\n", i);
				is_valid = 0;
			}
		}

		if (i > 89) {
			if (!graph__is_start(g, i, WHITE)) {
				printf("   %zu is not starting area of WHITE\n", i);
				is_valid = 0;
			}
		} else {
			if (graph__is_start(g, i, WHITE)) {
				printf("   %zu is starting area of WHITE\n", i);
				is_valid = 0;
			}
		}
	}

	graph__free(g);
	return is_valid;
}

int test_graph__toric()
{
	int is_valid = 1;
	struct graph_t* g = graph__toric(12);
	enum dir_t expected;
	for (size_t i = 0; i < 128; i++)
		for (size_t j = 0; j < 128; j++) {
			if (i < 48) {
				if (i % 12 != 0 && j == (i - 1)) {
					expected = WEST;
				} else if (i % 12 != 11 && j == (i + 1)) {
					expected = EAST;
				} else if (i > 11 && j == (i - 12)) {
					expected = NORTH;
				} else if ((i < 40 && j == (i + 12)) || (i > 43 && i < 48 && j == (i + 20))) {
					expected = SOUTH;
				} else
					expected = NONE;
			} else if (i < 80) {
				if (i % 4 != 0 && j == (i - 1)) {
					expected = WEST;
				} else if (i % 4 != 3 && j == (i + 1)) {
					expected = EAST;
				} else if ((((i > 51 && i < 64) || i > 67) && j == (i - 4)) || (i < 52 && j == (i - 12))
						   || (i > 63 && i < 68 && j == (i - 20))) {
					expected = NORTH;
				} else if (((i < 60 || (i > 63 && i < 76)) && j == (i + 4)) || (i > 59 && i < 64 && j == (i + 20))
						   || (i > 75 && j == (i + 12))) {
					expected = SOUTH;
				} else
					expected = NONE;
			} else {
				if (i % 12 != 8 && j == (i - 1)) {
					expected = WEST;
				} else if (i % 12 != 7 && j == (i + 1)) {
					expected = EAST;
				} else if ((i < 84 && j == (i - 20)) || (((i > 87 && i < 92) || i > 91) && j == (i - 12))) {
					expected = NORTH;
				} else if (i < 120 && j == (i + 12)) {
					expected = SOUTH;
				} else
					expected = NONE;
			}
			if ((graph__get_edge(g, (struct edge_t) {i, j}) != expected)) {
				printf("   Wrong value for %zu & %zu, got %u, expected %u\n", i, j, graph__get_edge(g, (struct edge_t) {i, j}), expected);
				is_valid = 0;
			}
		}
	for (size_t i = 0; i < 128; i++) {
		if (i < 12) {
			if (!graph__is_start(g, i, BLACK)) {
				printf("   %zu is not starting area of BLACK\n", i);
				is_valid = 0;
			}
		} else {
			if (graph__is_start(g, i, BLACK)) {
				printf("   %zu is starting area of BLACK\n", i);
				is_valid = 0;
			}
		}

		if (i > 115) {
			if (!graph__is_start(g, i, WHITE)) {
				printf("   %zu is not starting area of WHITE\n", i);
				is_valid = 0;
			}
		} else {
			if (graph__is_start(g, i, WHITE)) {
				printf("   %zu is starting area of WHITE\n", i);
				is_valid = 0;
			}
		}
	}

	graph__free(g);
	return is_valid;
}

int test_graph__s_shaped()
{
	int is_valid = 1;
	struct graph_t* g = graph__s_shaped(15);
	enum dir_t expected;
	for (size_t i = 0; i < 117; i++)
		for (size_t j = 0; j < 117; j++) {
			if (i < 9) {
				if (i % 3 != 0 && j == (i - 1)) {
					expected = WEST;
				} else if (i % 3 != 2 && j == (i + 1)) {
					expected = EAST;
				} else if (i > 2 && j == (i - 3)) {
					expected = NORTH;
				} else if (j == (i + 3)) {
					expected = SOUTH;
				} else
					expected = NONE;
			} else if (i < 54) {
				if (i % 15 != 9 && j == (i - 1)) {
					expected = WEST;
				} else if (i % 15 != 8 && j == (i + 1)) {
					expected = EAST;
				} else if ((i > 23 && j == (i - 15)) || (i < 12 && j == (i - 3))) {
					expected = NORTH;
				} else if ((i < 39 && j == (i + 15)) || (i > 50 && j == (i + 3))) {
					expected = SOUTH;
				} else
					expected = NONE;
			} else if (i < 63) {
				if (i % 3 != 0 && j == (i - 1)) {
					expected = WEST;
				} else if (i % 3 != 2 && j == (i + 1)) {
					expected = EAST;
				} else if (j == (i - 3)) {
					expected = NORTH;
				} else if ((i < 60 && j == (i + 3)) || (i > 59 && j == (i + 15))) {
					expected = SOUTH;
				} else
					expected = NONE;
			} else if (i < 108) {
				if (i % 15 != 3 && j == (i - 1)) {
					expected = WEST;
				} else if (i % 15 != 2 && j == (i + 1)) {
					expected = EAST;
				} else if ((i > 74 && j == (i - 15))) {
					expected = NORTH;
				} else if ((i < 96 && j == (i + 15))) {
					expected = SOUTH;
				} else
					expected = NONE;
			} else {
				if (i % 3 != 0 && j == (i - 1)) {
					expected = WEST;
				} else if (i % 3 != 2 && j == (i + 1)) {
					expected = EAST;
				} else if ((i < 111 && j == (i - 15)) || (i > 110 && j == (i - 3))) {
					expected = NORTH;
				} else if ((i < 141 && j == (i + 3))) {
					expected = SOUTH;
				} else
					expected = NONE;
			}
			if ((graph__get_edge(g, (struct edge_t) {i, j}) != expected)) {
				printf("   Wrong value for %zu & %zu, got %u, expected %u\n", i, j, graph__get_edge(g, (struct edge_t) {i, j}), expected);
				is_valid = 0;
			}
		}
	for (size_t i = 0; i < 117; i++) {
		if (i < 3) {
			if (!graph__is_start(g, i, BLACK)) {
				printf("   %zu is not starting area of BLACK\n", i);
				is_valid = 0;
			}
		} else {
			if (graph__is_start(g, i, BLACK)) {
				printf("   %zu is starting area of BLACK\n", i);
				is_valid = 0;
			}
		}

		if (i > 113) {
			if (!graph__is_start(g, i, WHITE)) {
				printf("   %zu is not starting area of WHITE\n", i);
				is_valid = 0;
			}
		} else {
			if (graph__is_start(g, i, WHITE)) {
				printf("   %zu is starting area of WHITE\n", i);
				is_valid = 0;
			}
		}
	}

	graph__free(g);
	return is_valid;
}

int test_graph__h_shaped()
{
	int is_valid = 1;
	struct graph_t* g = graph__h_shaped(12);
	enum dir_t expected;
	for (size_t i = 0; i < 112; i++)
		for (size_t j = 0; j < 112; j++) {
			if (i < 48) {
				if (i % 4 != 0 && j == (i - 1)) {
					expected = WEST;
				} else if ((i % 4 != 3 && j == (i + 1)) || (i / 4 > 3 && i / 4 < 8 && i % 4 == 3 && j == (i + 29))) {
					expected = EAST;
				} else if (i > 3 && j == (i - 4)) {
					expected = NORTH;
				} else if (i < 44 && j == (i + 4)) {
					expected = SOUTH;
				} else
					expected = NONE;
			} else if (i < 64) {
				if ((i % 4 != 0 && j == (i - 1)) || (i % 4 == 0 && j == (i - 29))) {
					expected = WEST;
				} else if ((i % 4 != 3 && j == (i + 1)) || (i % 4 == 3 && j == (i + 29))) {
					expected = EAST;
				} else if (i > 51 && j == (i - 4)) {
					expected = NORTH;
				} else if (i < 60 && j == (i + 4)) {
					expected = SOUTH;
				} else
					expected = NONE;
			} else {
				if ((i % 4 != 0 && j == (i - 1)) || (i / 4 > 19 && i / 4 < 24 && i % 4 == 0 && j == (i - 29))) {
					expected = WEST;
				} else if (i % 4 != 3 && j == (i + 1)) {
					expected = EAST;
				} else if (i > 67 && j == (i - 4)) {
					expected = NORTH;
				} else if (i < 108 && j == (i + 4)) {
					expected = SOUTH;
				} else
					expected = NONE;
			}
			if ((graph__get_edge(g, (struct edge_t) {i, j}) != expected)) {
				printf("   Wrong value for %zu & %zu, got %u, expected %u\n", i, j, graph__get_edge(g, (struct edge_t) {i, j}), expected);
				is_valid = 0;
			}
		}

	for (size_t i = 0; i < 112; i++) {
		if (i < 4 || (i > 63 && i < 68)) {
			if (!graph__is_start(g, i, BLACK)) {
				printf("   %zu is not starting area of BLACK\n", i);
				is_valid = 0;
			}
		} else {
			if (graph__is_start(g, i, BLACK)) {
				printf("   %zu is starting area of BLACK\n", i);
				is_valid = 0;
			}
		}

		if ((i > 43 && i < 48) || i > 107) {
			if (!graph__is_start(g, i, WHITE)) {
				printf("   %zu is not starting area of WHITE\n", i);
				is_valid = 0;
			}
		} else {
			if (graph__is_start(g, i, WHITE)) {
				printf("   %zu is starting area of WHITE\n", i);
				is_valid = 0;
			}
		}
	}

	graph__free(g);
	return is_valid;
}

int test_graph__claim_node()
{
	struct graph_t* g = graph__square(10);
	struct graph_t* g_check = graph__square(10);
	int is_valid = 1;

	gsl_spmatrix_uint_set(g_check->o, BLACK, 55, 1);
	graph__claim_node(g, 55, BLACK);

	if (!gsl_spmatrix_uint_equal(g->t, g_check->t)) {
		printf("   t not equal\n");
		is_valid = 0;
	}
	if (!gsl_spmatrix_uint_equal(g->o, g_check->o)) {
		printf("   o not equal\n");
		is_valid = 0;
	}

	graph__free(g_check);
	graph__free(g);
	return is_valid;
}

int test_graph__unclaim_node()
{
	struct graph_t* g = graph__square(10);
	struct graph_t* g_check = graph__square(10);
	int is_valid = 1;

	gsl_spmatrix_uint_set(g_check->o, BLACK, 55, 0);
	gsl_spmatrix_uint_set(g->o, BLACK, 55, 1);
	graph__unclaim_node(g, 55, BLACK);

	if (!gsl_spmatrix_uint_equal(g->t, g_check->t)) {
		printf("   t not equal\n");
		is_valid = 0;
	}
	if (!gsl_spmatrix_uint_equal(g->o, g_check->o)) {
		printf("   o not equal\n");
		is_valid = 0;
	}

	graph__free(g_check);
	graph__free(g);
	return is_valid;
}

int test_graph__is_start()
{
	struct graph_t* g = graph__square(10);
	int is_valid = 1;

	if (graph__is_start(g, 55, BLACK)) {
		printf("   Starting zone detected.\n");
		is_valid = 0;
	}

	gsl_spmatrix_uint_set(g->o, BLACK, 55, 1);

	if (!graph__is_start(g, 55, BLACK)) {
		printf("   Starting zone undetected.\n");
		is_valid = 0;
	}

	graph__free(g);
	return is_valid;
}
