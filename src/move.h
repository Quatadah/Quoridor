#ifndef _QUOR_MOVE_H_
#define _QUOR_MOVE_H_

#include <limits.h>
#include <stddef.h>
#include <stdint.h>

/* Players */
enum color_t { BLACK = 0, WHITE = 1, NO_COLOR = 2 };

/* Types of moves */
enum movetype_t { WALL = 0, MOVE = 1, NO_TYPE = 2 };

/* Ids of vertices are size_t between 0 and the number of vertices of - 1 */

struct edge_t {
	size_t fr, to;
}; // The ids of two vertices

/* Positions */
struct move_t {
	size_t m;			// The id of a vertex
	struct edge_t e[2]; // The two edges for the wall, set to -1 if not applicable
	enum movetype_t t;	// The type of the move
	enum color_t c;		// The color of the player
};

/* A special edge used to specify that this edge should not be used */
static inline struct edge_t no_edge()
{
	return (struct edge_t) {SIZE_MAX, SIZE_MAX};
}

/* A matcher for the special edges */
static inline int is_no_edge(const struct edge_t e)
{
	return (e.fr == SIZE_MAX) && (e.to == SIZE_MAX);
}

#endif // _QUOR_MOVE_H_
