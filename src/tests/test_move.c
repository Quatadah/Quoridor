#include "test_move.h"
#include <stdio.h>

int test_no_edge()
{
	struct edge_t edge = no_edge();
	return edge.fr == SIZE_MAX && edge.to == SIZE_MAX;
}

int test_is_no_edge()
{
	int is_valid = 1;
	struct edge_t edge = no_edge();
	if (!is_no_edge(edge)) {
		printf("   no_edge isn't a edge.\n");
		is_valid = 0;
	}

	edge = (struct edge_t) {18, 21};
	if (is_no_edge(edge)) {
		printf("   {18,21} isn't an edge.\n");
		is_valid = 0;
	}

	return is_valid;
}
