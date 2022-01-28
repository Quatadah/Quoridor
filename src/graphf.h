#ifndef _QUOR_GRAPHF_H_
#define _QUOR_GRAPHF_H_

#include "graph.h"

enum dir_t { NONE = 0, NORTH = 1, SOUTH = 2, WEST = 3, EAST = 4 };

/** \brief Creates a graph with no edges
 * \param num_vertices The number of nodes in the graph
 * \return A graph with num_vertices nodes and no edges
 *
 * The graph returned by this function must be freed at the end
 * using the graph__free function.
 */
struct graph_t* graph__init(size_t num_vertices);

/** \brief Makes a deep copy of a graph
 * \param g The graph to be copied
 * \return A deep copy of the graph g
 *
 * The graph returned by this function must be freed at the end
 * using the graph__free function.
 */
struct graph_t* graph__copy(struct graph_t* g);

/** \brief Claims a node for a player.
 * \param g The graph to be modified
 * \param s The node to be claimed
 * \param c The color of the player who will claim the node
 *
 * Claiming a node means changing the values in the matrix "o" of the graph.
 */
void graph__claim_node(struct graph_t* g, size_t s, enum color_t id);

/** \brief Unclaims a node for a player.
 * \param g The graph to be modified
 * \param s The node to be claimed
 * \param c The color of the player who will claim the node
 *
 * Claiming a node means changing the values in the matrix "o" of the graph.
 */
void graph__unclaim_node(struct graph_t* g, size_t s, enum color_t id);

/** \brief Tells wether the node has been clamed by the player
 * \param g The graph to be modified
 * \param s The node
 * \param c The color of the player who will claim the node
 *
 * It reads the matrix value at line id and column s.
 */
unsigned int graph__is_start(struct graph_t* g, size_t s, enum color_t id);

/** \brief Connects two nodes.
 * \param g The graph to be modified
 * \param s The edge containing the two nodes connected
 * \param d The direction of the connection
 *
 * This function connects the nodes both ways.
 * Connecting node i to the NORTH of node j connects node j to the SOUTH of node i aswell
 */
void graph__set_edge(struct graph_t* g, struct edge_t t, enum dir_t d);

/**
 * \brief Tells in which direction two nodes are connected
 *
 * \param g The graph to be analyzed
 * \param t The edge representing the two nodes
 * \return enum dir_t, the connection direction between points
 */
enum dir_t graph__get_edge(struct graph_t* g, struct edge_t t);

/**
 * \brief Returns the neighours of node n in direction dir
 *
 * \param g The graph to be analyzed
 * \param n The node whom neighbour has to be found
 * \param dir The direction the neighbour has to be found
 * \return SIZE_MAX if there is no root in this direction or the id of the neighbour node
 */
size_t graph__get_neighbour(struct graph_t* g, size_t n, enum dir_t dir);

/**
 * \brief Returns all neighbours of a node (max 4)
 *
 * \param g The graph to be analyzed
 * \param n The ID the node whom neighbours have to be found
 * \return A list (length 4) of all nodes that are neighbours of n in g.
 * Remaining values are SIZE_MAX if there is less than 4 neighbours.
 */
void graph__get_all_neighbours(struct graph_t* g, size_t n, size_t* neighbours);

/** \brief Frees the memory allocated to a graph
 * \param g The graph to be freed
 */
void graph__free(struct graph_t* g);

/** \brief Creates a square shaped graph
 * \param n The size of one side of the square
 * \return A square graph of n x n nodes
 *
 * The graph returned by this function must be freed at the end
 * using the graph__free function.
 */
struct graph_t* graph__square(size_t n);

/** \brief Creates a toric shaped graph
 * \param n The size of one side of the square
 * \return A toric graph of 2/3 n x n nodes
 *
 * The graph returned by this function must be freed at the end
 * using the graph__free function.
 */
struct graph_t* graph__toric(size_t n);

/** \brief List all staring positiion for player id in graph g
 * \param g The graph analyzed
 * \param id id of player
 * \return Array of all staring positiion for player id in graph g
 *
 * The array returned by this function must be freed
 */
size_t* graph__starting_positions(struct graph_t* g, enum color_t id);

/** \brief Creates a S shaped graph
 * \param n The size of one side of the square
 * \return A S shaped graph
 *
 * The graph returned by this function must be freed at the end
 * using the graph__free function.
 */
struct graph_t* graph__s_shaped(size_t n);

/** \brief Creates a h shaped graph
 * \param n The size of one side of the square
 * \return A H shaped graph
 *
 * The graph returned by this function must be freed at the end
 * using the graph__ free function.
 */
struct graph_t* graph__h_shaped(size_t n);

#endif // _QUOR_GRAPHF_H_
