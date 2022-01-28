#ifndef __CLIENT_COMMON_H__
#define __CLIENT_COMMON_H__

#include "player.h"
#include "board.h"

#define UNUSED(x) (void)(x)

extern const char* name;

extern struct board_t b;
extern enum color_t client_id, other_id;

struct distdata {
	size_t dist, next;
	int valid;
};

/**
 * \brief (Implementation dependant) Returns the next move computed by client
 *
 * \return next move computed by client
 */
struct move_t local_move(struct move_t* previous_move);

/**
 * \brief List the possible next positions if the client of id id was on starting_pos position
 *
 * \param bo The board
 * \param starting_position The player's position
 * \param id The client id
 *
 * \return a list of vertex (possible places to move)
 */
void possible_displacements(struct board_t* bo, size_t starting_pos, int id, size_t* displace);

/**
 * \brief Calculate distance between player and its winning zone (+ gives next case to go)
 *
 * \param bo The board
 * \param id The client id
 *
 * \return struct distdata containing all data necessary to go to the next vertex near to the winning zone
 */
struct distdata calc_distance(struct board_t* bo, enum color_t id);

/**
 * \brief If the client needs some sort of initialization
 *
 */

void local_initialize();

/**
 * \brief If the client needs some sort of initialization
 *
 */

void local_finalize();

#endif // __CLIENT_COMMON_H__
