#include <stdio.h>

#include "movef.h"

void print_move(struct move_t* m, const char* name, unsigned int round)
{
	printf("ROUND %u : %s ", round, name);
	switch (m->t) {
		case MOVE:
			printf("MOVED to position %lu.\n", m->m);
			break;

		case WALL:
			printf("placed a WALL cutting edges : (%lu<->%lu) and (%lu<->%lu).\n", m->e[0].to, m->e[0].fr, m->e[1].to, m->e[1].fr);
			break;

		case NO_TYPE:
			printf("did a NO_TYPE move.\n");
			break;

		default:
			printf("did a move that doesn't exist.\n");
			break;
	}
}
