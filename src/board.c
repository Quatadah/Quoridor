#include "board.h"
#include "graph.h"

int board__apply_move(struct board_t* b, const struct move_t* m)
{
	if (!board__check_move(b, m))
		return 0;
	switch (m->t) {
		case MOVE:
			b->pos[m->c] = m->m;
			break;

		case WALL:
			graph__set_edge(b->graph, m->e[0], 0);
			graph__set_edge(b->graph, m->e[1], 0);

			b->num_walls[m->c]--;
			break;

		case NO_TYPE:
			break;
	}
	return 1;
}

void board__init(struct board_t* b, struct graph_t* g)
{
	b->graph = g;
	b->graph_beginning = graph__copy(g);
	size_t num_walls = b->graph->t->nz / 2;
	num_walls = (num_walls % 15 == 0) ? num_walls / 15 : num_walls / 15 + 1;
	b->num_walls[BLACK] = num_walls;
	b->num_walls[WHITE] = num_walls;
	b->pos[BLACK] = SIZE_MAX;
	b->pos[WHITE] = SIZE_MAX;
}

int board__check_move(struct board_t* b, const struct move_t* m)
{
	switch (m->t) {
		case MOVE: {
			if (m->m >= b->graph->num_vertices || m->m == b->pos[BLACK] || m->m == b->pos[WHITE])
				return 0;
			if (b->pos[m->c] == SIZE_MAX)
				return graph__is_start(b->graph, m->m, m->c);
			if (graph__get_edge(b->graph, (struct edge_t) {b->pos[m->c], m->m}))
				return 1;
			if (b->pos[m->c == WHITE ? BLACK : WHITE] == SIZE_MAX)
				return 0;
			enum dir_t direction1 = graph__get_edge(b->graph, (struct edge_t) {b->pos[m->c], b->pos[m->c == WHITE ? BLACK : WHITE]});
			enum dir_t direction2 = graph__get_edge(b->graph, (struct edge_t) {b->pos[m->c == WHITE ? BLACK : WHITE], m->m});
			if (!direction1 || !direction2)
				return 0;
			if (direction1 == direction2)
				return 1;
			for (size_t i = 0; i < b->graph->num_vertices; i++)
				if (graph__get_edge(b->graph, (struct edge_t) {b->pos[m->c == WHITE ? BLACK : WHITE], i}) == direction1)
					return 0;
			return 1;
		}
		case WALL: {
			if (b->num_walls[m->c] <= 0)
				return 0;
			enum dir_t direction0 = graph__get_edge(b->graph, m->e[0]);
			enum dir_t direction1 = graph__get_edge(b->graph, m->e[1]);
			if (!direction0 || !direction1)
				return 0;

			enum dir_t inv_direction0 = direction0 > SOUTH ? (direction0 == EAST ? WEST : EAST) : (direction0 == NORTH ? SOUTH : NORTH);
			if (direction0 == direction1) {
				enum dir_t direction = graph__get_edge(b->graph, (struct edge_t) {m->e[0].fr, m->e[1].fr});
				if (direction0 == NORTH || direction0 == SOUTH) {
					if (direction == EAST || direction == WEST)
						return (b->pos[BLACK] == SIZE_MAX || board__accessible_end(b, BLACK, m))
							   && (b->pos[WHITE] == SIZE_MAX || board__accessible_end(b, WHITE, m));
					else
						direction = graph__get_edge(b->graph, (struct edge_t) {m->e[0].to, m->e[1].to});
					if (direction == EAST || direction == WEST)
						return (b->pos[BLACK] == SIZE_MAX || board__accessible_end(b, BLACK, m))
							   && (b->pos[WHITE] == SIZE_MAX || board__accessible_end(b, WHITE, m));
				}
				if (direction0 == EAST || direction0 == WEST) {
					if (direction == SOUTH || direction == NORTH)
						return (b->pos[BLACK] == SIZE_MAX || board__accessible_end(b, BLACK, m))
							   && (b->pos[WHITE] == SIZE_MAX || board__accessible_end(b, WHITE, m));
					else
						direction = graph__get_edge(b->graph, (struct edge_t) {m->e[0].to, m->e[1].to});
					if (direction == SOUTH || direction == NORTH)
						return (b->pos[BLACK] == SIZE_MAX || board__accessible_end(b, BLACK, m))
							   && (b->pos[WHITE] == SIZE_MAX || board__accessible_end(b, WHITE, m));
				}
				struct edge_t to_check = {graph__get_neighbour(b->graph_beginning, m->e[0].to, direction0),
										  graph__get_neighbour(b->graph_beginning, m->e[1].to, direction0)};
				int nb_block = 0;
				while (to_check.fr < b->graph->num_vertices && to_check.to < b->graph->num_vertices
					   && graph__get_edge(b->graph_beginning, to_check) != NONE && graph__get_edge(b->graph, to_check) == NONE) {
					nb_block++;
					to_check.fr = graph__get_neighbour(b->graph_beginning, to_check.fr, direction0);
					to_check.to = graph__get_neighbour(b->graph_beginning, to_check.to, direction0);
				}
				return nb_block % 2 && (b->pos[BLACK] == SIZE_MAX || board__accessible_end(b, BLACK, m))
					   && (b->pos[WHITE] == SIZE_MAX || board__accessible_end(b, WHITE, m));
			} else if (direction1 == inv_direction0) {
				enum dir_t direction = graph__get_edge(b->graph, (struct edge_t) {m->e[0].fr, m->e[1].to});
				if (direction0 == NORTH || direction0 == SOUTH) {
					if (direction == EAST || direction == WEST)
						return (b->pos[BLACK] == SIZE_MAX || board__accessible_end(b, BLACK, m))
							   && (b->pos[WHITE] == SIZE_MAX || board__accessible_end(b, WHITE, m));
					else
						direction = graph__get_edge(b->graph, (struct edge_t) {m->e[0].to, m->e[1].fr});
					if (direction == EAST || direction == WEST)
						return (b->pos[BLACK] == SIZE_MAX || board__accessible_end(b, BLACK, m))
							   && (b->pos[WHITE] == SIZE_MAX || board__accessible_end(b, WHITE, m));
				}
				if (direction0 == EAST || direction0 == WEST) {
					if (direction == SOUTH || direction == NORTH)
						return (b->pos[BLACK] == SIZE_MAX || board__accessible_end(b, BLACK, m))
							   && (b->pos[WHITE] == SIZE_MAX || board__accessible_end(b, WHITE, m));
					else
						direction = graph__get_edge(b->graph, (struct edge_t) {m->e[0].to, m->e[1].fr});
					if (direction == SOUTH || direction == NORTH)
						return (b->pos[BLACK] == SIZE_MAX || board__accessible_end(b, BLACK, m))
							   && (b->pos[WHITE] == SIZE_MAX || board__accessible_end(b, WHITE, m));
				}
				struct edge_t to_check = {graph__get_neighbour(b->graph_beginning, m->e[0].to, direction0),
										  graph__get_neighbour(b->graph_beginning, m->e[1].fr, direction0)};
				int nb_block = 0;
				while (to_check.fr < b->graph->num_vertices && to_check.to < b->graph->num_vertices
					   && graph__get_edge(b->graph_beginning, to_check) != NONE && graph__get_edge(b->graph, to_check) == NONE) {
					nb_block++;
					to_check.fr = graph__get_neighbour(b->graph_beginning, to_check.fr, direction0);
					to_check.to = graph__get_neighbour(b->graph_beginning, to_check.to, direction0);
				}
				return nb_block % 2 && (b->pos[BLACK] == SIZE_MAX || board__accessible_end(b, BLACK, m))
					   && (b->pos[WHITE] == SIZE_MAX || board__accessible_end(b, WHITE, m));
			}
			return 0;
		}

		case NO_TYPE:
			return 1;
	}
	return 0;
}

int board__accessible_end(struct board_t* bo, int id, const struct move_t* M)
{
	char* f = (char*)calloc(bo->graph->num_vertices, sizeof(char));
	size_t* v = (size_t*)malloc(bo->graph->num_vertices * sizeof(size_t));

	for (size_t i = 0; i < bo->graph->num_vertices; i++) {
		v[i] = SIZE_MAX;
	}

	int s = 0;
	int e = 1;
	v[0] = bo->pos[id];
	f[bo->pos[id]] = 1;
	size_t i = v[s];
	size_t neighbours[4];
	while (s != e) {
		graph__get_all_neighbours(bo->graph, i, neighbours);
		for (size_t j = 0; j < 4; j++) {
			if (neighbours[j] == SIZE_MAX)
				break;

			if (!((M->e[0].fr == i && M->e[0].to == neighbours[j]) || (M->e[1].fr == i && M->e[1].to == neighbours[j])
				  || (M->e[0].to == i && M->e[0].fr == neighbours[j]) || (M->e[1].to == i && M->e[1].fr == neighbours[j]))) {
				if (f[neighbours[j]] == 0) {
					if (graph__is_start(bo->graph, neighbours[j], (id == BLACK) ? WHITE : BLACK)) {
						free(v);
						free(f);
						return 1;
					}

					v[e] = neighbours[j];
					f[neighbours[j]] = 1;
					e++;
				}
			}
		}

		f[i] = 1;
		s++;
		i = v[s];
	}
	free(v);
	free(f);
	return 0;
}

int board__are_equal(struct board_t* b1, struct board_t* b2)
{
	int is_valid = 1;

	if (!gsl_spmatrix_uint_equal(b1->graph->t, b2->graph->t)) {
		printf("   t not equal\n");
		is_valid = 0;
	}
	if (!gsl_spmatrix_uint_equal(b1->graph->o, b2->graph->o)) {
		printf("   o not equal\n");
		is_valid = 0;
	}
	if (!gsl_spmatrix_uint_equal(b1->graph_beginning->t, b2->graph_beginning->t)) {
		printf("   t_beginning not equal\n");
		is_valid = 0;
	}
	if (!gsl_spmatrix_uint_equal(b1->graph_beginning->o, b2->graph_beginning->o)) {
		printf("   o_beginning not equal\n");
		is_valid = 0;
	}
	if (b1->num_walls[0] != b2->num_walls[0]) {
		printf("   num_walls[0] not equal\n");
		is_valid = 0;
	}
	if (b1->num_walls[1] != b2->num_walls[1]) {
		printf("   num_walls[1] not equal\n");
		is_valid = 0;
	}
	if (b1->pos[0] != b2->pos[0]) {
		printf("   pos[0] not equal\n");
		is_valid = 0;
	}
	if (b1->pos[1] != b2->pos[1]) {
		printf("   pos[1] not equal\n");
		is_valid = 0;
	}
	return is_valid;
}

struct board_t* board__copy(struct board_t* b)
{
	struct board_t* b_cpy = (struct board_t*)malloc(sizeof(struct board_t));

	b_cpy->graph = graph__copy(b->graph);
	b_cpy->graph_beginning = graph__copy(b->graph_beginning);
	b_cpy->pos[BLACK] = b->pos[BLACK];
	b_cpy->pos[WHITE] = b->pos[WHITE];
	b_cpy->num_walls[BLACK] = b->num_walls[BLACK];
	b_cpy->num_walls[WHITE] = b->num_walls[WHITE];

	return b_cpy;
}

void board__release(struct board_t* b)
{
	graph__free(b->graph);
	graph__free(b->graph_beginning);
}
