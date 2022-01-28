#ifndef __GRAPH_OPTIMIZED_H__
#define __GRAPH_OPTIMIZED_H__

#include "graph.h"
#include "board.h"
#include "client_common.h"

struct board_optimized {
	size_t* file;
	size_t* parents;
	size_t* distances;

	size_t (*neighbours)[4];
	size_t (*neighbours_beginning)[4];
	struct edge_t (*walls)[2];

	size_t* starting_positions[2];

	size_t pos[2];
	size_t num_walls[2];

	size_t num_vertices;
};

struct save_move_t {
	enum movetype_t t;
	size_t m;
	struct edge_t e[2];
	enum dir_t d[2];
	enum color_t c;
};

void board_optimized__initialize(struct graph_t* g, struct board_optimized* bo);
void board_optimized__release(struct board_optimized* bo);

/** \brief Changes a board to the state it should be after a move is played
 * \param b The board to be changed
 * \param m The move to be applied
 *
 * \result Boolean telling wether the move was applied
 * This function changes the board given as a parameter.
 */

struct save_move_t board_optimized__apply_move_with_save(struct board_optimized* bo, const struct move_t* m);
void board_optimized__apply_save(struct board_optimized* bo, const struct save_move_t* save);

/**
 * \brief Checks wether a move is applicable on the board according to the rules.
 *
 * \param b The board this move would be applied to
 * \param m The move to check
 * \return A boolean telling wether the move is correct
 */
int board_optimized__check_move(struct board_optimized* bo, const struct move_t* m);

void board_optimized__possible_displacements(struct board_optimized* bo, size_t starting_pos, int id, size_t* displace);

/**
 * \brief Calculate distance between player and its winning zone (+ gives next case to go)
 *
 * \param bo The board
 * \param id The client id
 *
 * \return struct distdata containing all data necessary to go to the next vertex near to the winning zone
 */
struct distdata board_optimized__calc_distance(struct board_optimized* bo, enum color_t id);

/**
 * \brief Checks wether the move blocks the possibility of a user to access his winning area
 *
 * \param bo The board this move would be applied to
 * \param id The ID of the user that would do this move
 * \param M The move to check
 * \return Boolean telling if the player can access his winning zone
 */
int board_optimized__accessible_end(struct board_optimized* bo, enum color_t id, const struct move_t* m);

/** \brief Tells wether the node has been clamed by the player
 * \param g The graph to be modified
 * \param s The node
 * \param c The color of the player who will claim the node
 *
 * It reads the matrix value at line id and column s.
 */
unsigned int board_optimized__is_start(struct board_optimized* bo, size_t s, enum color_t id);

/** \brief Connects two nodes.
 * \param g The graph to be modified
 * \param s The edge containing the two nodes connected
 * \param d The direction of the connection
 *
 * This function connects the nodes both ways.
 * Connecting node i to the NORTH of node j connects node j to the SOUTH of node i aswell
 */
void neighbours__set_edge(size_t (*ne)[4], struct edge_t t, enum dir_t d);

/**
 * \brief Tells in which direction two nodes are connected
 *
 * \param g The graph to be analyzed
 * \param t The edge representing the two nodes
 * \return enum dir_t, the connection direction between points
 */
enum dir_t neighbours__get_edge(size_t (*ne)[4], struct edge_t t);

/**
 * \brief Returns the neighours of node n in direction dir
 *
 * \param g The graph to be analyzed
 * \param n The node whom neighbour has to be found
 * \param dir The direction the neighbour has to be found
 * \return SIZE_MAX if there is no root in this direction or the id of the neighbour node
 */
size_t neighbours__get_neighbour(size_t (*ne)[4], size_t n, enum dir_t dir);

/**
 * \brief Returns all neighbours of a node (max 4)
 *
 * \param g The graph to be analyzed
 * \param n The ID the node whom neighbours have to be found
 * \return A list (length 4) of all nodes that are neighbours of n in g.
 * Remaining values are SIZE_MAX if there is less than 4 neighbours.
 */
void neighbours__get_all_neighbours(size_t (*ne)[4], size_t n, size_t* neighbours);

/** \brief List all staring positiion for player id in graph g
 * \param g The graph analyzed
 * \param id id of player
 * \return Array of all staring positiion for player id in graph g
 *
 * The array returned by this function must be freed
 */
size_t* board_optimized__starting_positions(struct board_optimized* bo, enum color_t id);

#endif // __GRAPH_OPTIMIZED_H__
