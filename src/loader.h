#ifndef __LOADER_H__
#define __LOADER_H__

#include "graph.h"

typedef char const* (*pfn_get_player_name)(void);
typedef void (*pfn_initialize)(enum color_t id, struct graph_t* graph, size_t num_walls);
typedef struct move_t (*pfn_play)(struct move_t previous_move);
typedef void (*pfn_finalize)(void);

struct client {
	pfn_get_player_name get_player_name;
	pfn_initialize initialize;
	pfn_play play;
	pfn_finalize finalize;

	void* library;
};

/**
 * \brief Opens the library of a specified client and load its functions.
 *
 * \param name string to the client .so path
 * \param client a pointer to a struct client* where it will be loaded. It has to be allocated
 * \return a boolean value telling if it worked
 */
int load_client(const char* name, struct client* client);

/**
 * \brief Close the client library.
 *
 * \param client The clent which has to be unloaded
 * \return a boolean value telling if it worked
 */
int unload_client(struct client* client);

#endif // __LOADER_H__
