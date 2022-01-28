#include "graph_optimized.h"

void board_optimized__initialize(struct graph_t* g, struct board_optimized* bo)
{
	bo->file = (size_t*)malloc(g->num_vertices * sizeof(size_t));
	bo->parents = (size_t*)malloc(g->num_vertices * sizeof(size_t));
	bo->distances = (size_t*)malloc(g->num_vertices * sizeof(size_t));

	bo->neighbours = malloc(sizeof(size_t) * g->num_vertices * 4);
	for (size_t i = 0; i < g->num_vertices; i++) {
		bo->neighbours[i][0] = SIZE_MAX;
		bo->neighbours[i][1] = SIZE_MAX;
		bo->neighbours[i][2] = SIZE_MAX;
		bo->neighbours[i][3] = SIZE_MAX;
	}

	for (size_t i = 0; i < g->t->nz; i++) {
		if (g->t->data[i] != 0)
			bo->neighbours[g->t->i[i]][g->t->data[i] - 1] = g->t->p[i];
	}

	bo->neighbours_beginning = malloc(sizeof(size_t) * g->num_vertices * 4);
	for (size_t i = 0; i < g->num_vertices; i++) {
		bo->neighbours_beginning[i][0] = SIZE_MAX;
		bo->neighbours_beginning[i][1] = SIZE_MAX;
		bo->neighbours_beginning[i][2] = SIZE_MAX;
		bo->neighbours_beginning[i][3] = SIZE_MAX;
	}

	for (size_t i = 0; i < g->t->nz; i++) {
		if (g->t->data[i] != 0)
			bo->neighbours_beginning[g->t->i[i]][g->t->data[i] - 1] = g->t->p[i];
	}

	bo->starting_positions[BLACK] = graph__starting_positions(g, BLACK);
	bo->starting_positions[WHITE] = graph__starting_positions(g, WHITE);

	size_t num_walls = g->t->nz / 2;
	num_walls = (num_walls % 15 == 0) ? num_walls / 15 : num_walls / 15 + 1;
	bo->num_walls[BLACK] = num_walls;
	bo->num_walls[WHITE] = num_walls;
	bo->pos[BLACK] = SIZE_MAX;
	bo->pos[WHITE] = SIZE_MAX;

	bo->num_vertices = g->num_vertices;

	size_t nb_walls = 0;
	bo->walls = malloc(sizeof(struct edge_t[2]) * (g->num_vertices * 2) * (g->num_vertices * 2 - 1) / 2 + 1);
	struct move_t move = {0, {{0, 0}, {0, 0}}, WALL, BLACK};

	for (size_t i = 0; i < g->t->nz; i++) {
		move.e[0].fr = g->t->i[i];
		move.e[0].to = g->t->p[i];
		for (size_t j = i + 1; j < g->t->nz; j++) {
			move.e[1].fr = g->t->i[j];
			move.e[1].to = g->t->p[j];
			if (board_optimized__check_move(bo, &move)) {
				bo->walls[nb_walls][0] = (struct edge_t) {g->t->i[i], g->t->p[i]};
				bo->walls[nb_walls][1] = (struct edge_t) {g->t->i[j], g->t->p[j]};
				nb_walls++;
			}
		}
	}
	bo->walls[nb_walls][0] = no_edge();
	bo->walls[nb_walls][1] = no_edge();
	nb_walls++;

	bo->walls = realloc(bo->walls, sizeof(struct edge_t[2]) * nb_walls);
}

void board_optimized__release(struct board_optimized* bo)
{
	free(bo->file);
	free(bo->parents);
	free(bo->distances);

	free(bo->neighbours);
	free(bo->neighbours_beginning);
	free(bo->walls);

	free(bo->starting_positions[0]);
	free(bo->starting_positions[1]);
}

struct save_move_t board_optimized__apply_move_with_save(struct board_optimized* bo, const struct move_t* m)
{
	struct save_move_t save;
	switch (m->t) {
		case MOVE:
			save.t = MOVE;
			save.c = m->c;
			save.m = bo->pos[m->c];
			bo->pos[m->c] = m->m;
			break;

		case WALL:
			save.t = WALL;
			save.c = m->c;
			save.e[0] = m->e[0];
			save.e[1] = m->e[1];
			save.d[0] = neighbours__get_edge(bo->neighbours, m->e[0]);
			save.d[1] = neighbours__get_edge(bo->neighbours, m->e[1]);

			neighbours__set_edge(bo->neighbours, m->e[0], 0);
			neighbours__set_edge(bo->neighbours, m->e[1], 0);

			bo->num_walls[m->c]--;
			break;

		case NO_TYPE:
			save.t = NO_TYPE;
			break;
	}
	return save;
}

void board_optimized__apply_save(struct board_optimized* bo, const struct save_move_t* save)
{
	switch (save->t) {
		case MOVE:
			bo->pos[save->c] = save->m;
			break;

		case WALL:
			neighbours__set_edge(bo->neighbours, save->e[0], save->d[0]);
			neighbours__set_edge(bo->neighbours, save->e[1], save->d[1]);

			bo->num_walls[save->c]++;
			break;

		case NO_TYPE:
			break;
	}
}

int board_optimized__check_move(struct board_optimized* bo, const struct move_t* m)
{
	switch (m->t) {
		case MOVE: {
			if (m->m >= bo->num_vertices || m->m == bo->pos[BLACK] || m->m == bo->pos[WHITE])
				return 0;
			if (bo->pos[m->c] == SIZE_MAX)
				return board_optimized__is_start(bo, m->m, m->c);
			if (neighbours__get_edge(bo->neighbours, (struct edge_t) {bo->pos[m->c], m->m}))
				return 1;
			if (bo->pos[m->c == WHITE ? BLACK : WHITE] == SIZE_MAX)
				return 0;
			enum dir_t direction1 =
				neighbours__get_edge(bo->neighbours, (struct edge_t) {bo->pos[m->c], bo->pos[m->c == WHITE ? BLACK : WHITE]});
			enum dir_t direction2 = neighbours__get_edge(bo->neighbours, (struct edge_t) {bo->pos[m->c == WHITE ? BLACK : WHITE], m->m});
			if (!direction1 || !direction2)
				return 0;
			if (direction1 == direction2)
				return 1;
			for (size_t i = 0; i < bo->num_vertices; i++)
				if (neighbours__get_edge(bo->neighbours, (struct edge_t) {bo->pos[m->c == WHITE ? BLACK : WHITE], i}) == direction1)
					return 0;
			return 1;
		}
		case WALL: {
			if (bo->num_walls[m->c] <= 0)
				return 0;
			enum dir_t direction0 = neighbours__get_edge(bo->neighbours, m->e[0]);
			enum dir_t direction1 = neighbours__get_edge(bo->neighbours, m->e[1]);
			if (!direction0 || !direction1)
				return 0;

			enum dir_t inv_direction0 = direction0 > SOUTH ? (direction0 == EAST ? WEST : EAST) : (direction0 == NORTH ? SOUTH : NORTH);
			if (direction0 == direction1) {
				enum dir_t direction = neighbours__get_edge(bo->neighbours, (struct edge_t) {m->e[0].fr, m->e[1].fr});
				if (direction0 == NORTH || direction0 == SOUTH) {
					if (direction == EAST || direction == WEST)
						return (bo->pos[BLACK] == SIZE_MAX || board_optimized__accessible_end(bo, BLACK, m))
							   && (bo->pos[WHITE] == SIZE_MAX || board_optimized__accessible_end(bo, WHITE, m));
					else
						direction = neighbours__get_edge(bo->neighbours, (struct edge_t) {m->e[0].to, m->e[1].to});
					if (direction == EAST || direction == WEST)
						return (bo->pos[BLACK] == SIZE_MAX || board_optimized__accessible_end(bo, BLACK, m))
							   && (bo->pos[WHITE] == SIZE_MAX || board_optimized__accessible_end(bo, WHITE, m));
				}
				if (direction0 == EAST || direction0 == WEST) {
					if (direction == SOUTH || direction == NORTH)
						return (bo->pos[BLACK] == SIZE_MAX || board_optimized__accessible_end(bo, BLACK, m))
							   && (bo->pos[WHITE] == SIZE_MAX || board_optimized__accessible_end(bo, WHITE, m));
					else
						direction = neighbours__get_edge(bo->neighbours, (struct edge_t) {m->e[0].to, m->e[1].to});
					if (direction == SOUTH || direction == NORTH)
						return (bo->pos[BLACK] == SIZE_MAX || board_optimized__accessible_end(bo, BLACK, m))
							   && (bo->pos[WHITE] == SIZE_MAX || board_optimized__accessible_end(bo, WHITE, m));
				}
				struct edge_t to_check = {neighbours__get_neighbour(bo->neighbours_beginning, m->e[0].to, direction0),
										  neighbours__get_neighbour(bo->neighbours_beginning, m->e[1].to, direction0)};
				int nb_block = 0;
				while (to_check.fr < bo->num_vertices && to_check.to < bo->num_vertices
					   && neighbours__get_edge(bo->neighbours_beginning, to_check) != NONE
					   && neighbours__get_edge(bo->neighbours, to_check) == NONE) {
					nb_block++;
					to_check.fr = neighbours__get_neighbour(bo->neighbours_beginning, to_check.fr, direction0);
					to_check.to = neighbours__get_neighbour(bo->neighbours_beginning, to_check.to, direction0);
				}
				return nb_block % 2 && (bo->pos[BLACK] == SIZE_MAX || board_optimized__accessible_end(bo, BLACK, m))
					   && (bo->pos[WHITE] == SIZE_MAX || board_optimized__accessible_end(bo, WHITE, m));
			} else if (direction1 == inv_direction0) {
				enum dir_t direction = neighbours__get_edge(bo->neighbours, (struct edge_t) {m->e[0].fr, m->e[1].to});
				if (direction0 == NORTH || direction0 == SOUTH) {
					if (direction == EAST || direction == WEST)
						return (bo->pos[BLACK] == SIZE_MAX || board_optimized__accessible_end(bo, BLACK, m))
							   && (bo->pos[WHITE] == SIZE_MAX || board_optimized__accessible_end(bo, WHITE, m));
					else
						direction = neighbours__get_edge(bo->neighbours, (struct edge_t) {m->e[0].to, m->e[1].fr});
					if (direction == EAST || direction == WEST)
						return (bo->pos[BLACK] == SIZE_MAX || board_optimized__accessible_end(bo, BLACK, m))
							   && (bo->pos[WHITE] == SIZE_MAX || board_optimized__accessible_end(bo, WHITE, m));
				}
				if (direction0 == EAST || direction0 == WEST) {
					if (direction == SOUTH || direction == NORTH)
						return (bo->pos[BLACK] == SIZE_MAX || board_optimized__accessible_end(bo, BLACK, m))
							   && (bo->pos[WHITE] == SIZE_MAX || board_optimized__accessible_end(bo, WHITE, m));
					else
						direction = neighbours__get_edge(bo->neighbours, (struct edge_t) {m->e[0].to, m->e[1].fr});
					if (direction == SOUTH || direction == NORTH)
						return (bo->pos[BLACK] == SIZE_MAX || board_optimized__accessible_end(bo, BLACK, m))
							   && (bo->pos[WHITE] == SIZE_MAX || board_optimized__accessible_end(bo, WHITE, m));
				}
				struct edge_t to_check = {neighbours__get_neighbour(bo->neighbours_beginning, m->e[0].to, direction0),
										  neighbours__get_neighbour(bo->neighbours_beginning, m->e[1].fr, direction0)};
				int nb_block = 0;
				while (to_check.fr < bo->num_vertices && to_check.to < bo->num_vertices
					   && neighbours__get_edge(bo->neighbours_beginning, to_check) != NONE
					   && neighbours__get_edge(bo->neighbours, to_check) == NONE) {
					nb_block++;
					to_check.fr = neighbours__get_neighbour(bo->neighbours_beginning, to_check.fr, direction0);
					to_check.to = neighbours__get_neighbour(bo->neighbours_beginning, to_check.to, direction0);
				}
				return nb_block % 2 && (bo->pos[BLACK] == SIZE_MAX || board_optimized__accessible_end(bo, BLACK, m))
					   && (bo->pos[WHITE] == SIZE_MAX || board_optimized__accessible_end(bo, WHITE, m));
			}
			return 0;
		}

		case NO_TYPE:
			return 1;
	}
	return 0;
}

void board_optimized__possible_displacements(struct board_optimized* bo, size_t starting_pos, int id, size_t* displace)
{
	size_t other_id = (id == BLACK) ? WHITE : BLACK;
	size_t neighbours[4];
	neighbours__get_all_neighbours(bo->neighbours, starting_pos, neighbours);
	size_t k = 0;

	for (size_t i = 0; i < 4; i++) {
		if (neighbours[i] == bo->pos[id])
			continue;

		if (neighbours[i] == SIZE_MAX)
			break;

		if (bo->pos[other_id] != SIZE_MAX && neighbours[i] == bo->pos[other_id]) {
			enum dir_t direction = neighbours__get_edge(bo->neighbours, (struct edge_t) {bo->pos[id], bo->pos[other_id]});
			size_t next = neighbours__get_neighbour(bo->neighbours, bo->pos[other_id], direction);
			if (next == SIZE_MAX) {
				enum dir_t d1 = (direction == NORTH || direction == SOUTH) ? EAST : NORTH;
				enum dir_t d2 = (direction == NORTH || direction == SOUTH) ? WEST : SOUTH;

				size_t nb = neighbours__get_neighbour(bo->neighbours, bo->pos[other_id], d1);
				if (nb != SIZE_MAX && nb != bo->pos[id]) {
					displace[k] = nb;
					k++;
				}
				nb = neighbours__get_neighbour(bo->neighbours, bo->pos[other_id], d2);
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

struct distdata board_optimized__calc_distance(struct board_optimized* bo, enum color_t id)
{
	enum color_t n_id = (id == BLACK) ? WHITE : BLACK;
	int start = 0;
	int end = 0;

	for (size_t i = 0; i < bo->num_vertices; i++) {
		bo->parents[i] = SIZE_MAX;
	}

	bo->parents[bo->pos[id]] = bo->pos[id];
	bo->file[0] = bo->pos[id];
	end++;
	bo->distances[bo->pos[id]] = 0;
	size_t neighbours[6];
	while (end > start) {
		size_t vertex = bo->file[start];
		if (bo->distances[vertex] == 0)
			board_optimized__possible_displacements(bo, vertex, id, neighbours);
		else
			neighbours__get_all_neighbours(bo->neighbours, vertex, neighbours);
		for (size_t j = 0; j < ((bo->distances[vertex] == 0) ? 5 : 4); j++) {
			if (neighbours[j] == SIZE_MAX)
				break;
			size_t n = neighbours[j];

			if (bo->parents[n] == SIZE_MAX) {
				bo->parents[n] = vertex;
				bo->file[end] = n;
				end++;
				bo->distances[n] = bo->distances[vertex] + 1;
				if (board_optimized__is_start(bo, n, n_id)) {
					size_t d = bo->distances[n];
					while (bo->parents[n] != bo->pos[id]) {
						n = bo->parents[n];
					}
					return (struct distdata) {d, n, 1};
				}
			}
		}
		start++;
	}
	return (struct distdata) {SIZE_MAX, SIZE_MAX, 0};
}

int board_optimized__accessible_end(struct board_optimized* bo, enum color_t id, const struct move_t* m)
{
	struct save_move_t save = board_optimized__apply_move_with_save(bo, m);
	int result = board_optimized__calc_distance(bo, id).valid;
	board_optimized__apply_save(bo, &save);
	return result;
}

// WARN perte en complexité !
unsigned int board_optimized__is_start(struct board_optimized* bo, size_t s, enum color_t id)
{
	size_t* current_pos = bo->starting_positions[id];
	while (*current_pos != SIZE_MAX) {
		if (*current_pos == s)
			return 1;
		current_pos++;
	}
	return 0;
}

void neighbours__set_edge(size_t (*ne)[4], struct edge_t t, enum dir_t d)
{
	enum dir_t ancient_dir = neighbours__get_edge(ne, t);
	if (ancient_dir == d)
		return;

	if (ancient_dir != NONE) {
		ne[t.fr][ancient_dir - 1] = SIZE_MAX;
		ne[t.to][(ancient_dir > SOUTH ? (ancient_dir == EAST ? WEST : EAST) : (ancient_dir == NORTH ? SOUTH : NORTH)) - 1] = SIZE_MAX;
	}

	if (d != NONE) {
		ne[t.fr][d - 1] = t.to;
		ne[t.to][(d > SOUTH ? (d == EAST ? WEST : EAST) : (d == NORTH ? SOUTH : NORTH)) - 1] = t.fr;
	}
}

enum dir_t neighbours__get_edge(size_t (*ne)[4], struct edge_t t)
{
	for (enum dir_t d = 0; d < 4; d++) {
		if (ne[t.fr][d] == t.to)
			return d + 1;
	}
	return NONE;
}

size_t neighbours__get_neighbour(size_t (*ne)[4], size_t n, enum dir_t dir)
{
	if (dir == NONE)
		return SIZE_MAX;
	return ne[n][dir - 1];
}

void neighbours__get_all_neighbours(size_t (*ne)[4], size_t n, size_t* neighbours)
{
	size_t nb = 0;
	for (enum dir_t d = 0; d < 4; d++) {
		if (ne[n][d] != SIZE_MAX) {
			neighbours[nb] = ne[n][d];
			nb++;
		}
	}
	if (nb < 4)
		neighbours[nb] = SIZE_MAX;
}

size_t* board_optimized__starting_positions(struct board_optimized* bo, enum color_t id)
{
	return bo->starting_positions[id];
}
