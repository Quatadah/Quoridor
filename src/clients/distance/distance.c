
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "client_common.h"
#include "player.h"
#include "move.h"
#include "graph.h"
#include "board.h"

const char* name = "Distance";

void local_initialize()
{
}

void local_finalize()
{
}

struct move_t local_move(struct move_t* previous_move)
{
	UNUSED(previous_move);
	size_t pos;
	if (b.pos[client_id] == SIZE_MAX) {
		size_t* validpos = graph__starting_positions(b.graph, client_id);
		size_t* currentpos = validpos;
		int end_pile = 1;
		size_t distance = INT_MAX;
		while (*currentpos != SIZE_MAX) {
			b.pos[client_id] = *currentpos;
			struct distdata dist = calc_distance(&b, client_id);
			if (dist.dist < distance) {
				end_pile = 1;
				validpos[0] = *currentpos;
				distance = dist.dist;
			} else if (dist.dist == distance) {
				validpos[end_pile] = *currentpos;
				end_pile++;
			}
			currentpos++;
		}
		b.pos[client_id] = SIZE_MAX;
		pos = validpos[rand() % end_pile];
		free(validpos);
	} else {
		pos = calc_distance(&b, client_id).next;
	}
	return (struct move_t) {pos, {no_edge(), no_edge()}, MOVE, client_id};
}
