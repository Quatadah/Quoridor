#ifndef _BOARD_H_
#define _BOARD_H_

#include "graphf.h"
#include "move.h"

struct board_t {
	struct graph_t* graph;
	struct graph_t* graph_beginning;
	size_t num_walls[2];
	size_t pos[2];
};

/** \brief Changes a board to the state it should be after a move is played
 * \param b The board to be changed
 * \param m The move to be applied
 *
 * \result Boolean telling wether the move was applied
 * This function changes the board given as a parameter.
 */

int board__apply_move(struct board_t* b, const struct move_t* m);

/**
 * \brief Initializer for a board
 *
 * \param b The board to be changed
 * \param n The size of the graph
 *
 * This function changes the board given as parameter.
 */
void board__init(struct board_t* b, struct graph_t* g);

/**
 * \brief Checks wether a move is applicable on the board according to the rules.
 *
 * \param b The board this move would be applied to
 * \param m The move to check
 * \return A boolean telling wether the move is correct
 */
int board__check_move(struct board_t* b, const struct move_t* m);

/**
 * \brief Checks wether the move blocks the possibility of a user to access his winning area
 *
 * \param bo The board this move would be applied to
 * \param id The ID of the user that would do this move
 * \param M The move to check
 * \return Boolean telling if the player can access his winning zone
 */
int board__accessible_end(struct board_t* bo, int id, const struct move_t* M);

/**
 * \brief Checks if boards content are equal
 *
 * \param b1 The first board
 * \param b2 The second board
 * \return Boolean telling wether board contents are equal
 */
int board__are_equal(struct board_t* b1, struct board_t* b2);

/**
 * \brief Makes a deep copy of a board
 *
 * \param b The board to be copied
 * \return A deep copy of the board
 * The copy must be free after use using the function board__release and a free.
 */
struct board_t* board__copy(struct board_t* b);

/**
 * \brief Frees graphs of board
 *
 * \param b The board copy to be released
 */
void board__release(struct board_t* b);

#endif // _BOARD_H_
