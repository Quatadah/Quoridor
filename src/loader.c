#include "loader.h"
#include <dlfcn.h>

int load_client(const char* name, struct client* client)
{
	client->library = dlopen(name, RTLD_LAZY);
	const char* error;
	if (!client->library) {
		if ((error = dlerror()))
			printf("%s\n", error);
		return 1;
	}

	client->get_player_name = (pfn_get_player_name)dlsym(client->library, "get_player_name");
	if ((error = dlerror())) {
		printf("Error while loading get_player_name for client %s : %s\n", name, error);
		return 2;
	}
	client->initialize = (pfn_initialize)dlsym(client->library, "initialize");
	if ((error = dlerror())) {
		printf("Error while loading initialize for client %s : %s\n", name, error);
		return 3;
	}
	client->play = (pfn_play)dlsym(client->library, "play");
	if ((error = dlerror())) {
		printf("Error while loading play for client %s : %s\n", name, error);
		return 4;
	}
	client->finalize = (pfn_finalize)dlsym(client->library, "finalize");
	if ((error = dlerror())) {
		printf("Error while loading finalize for client %s : %s\n", name, error);
		return 5;
	}

	return 0;
}

int unload_client(struct client* client)
{
	dlclose(client->library);
	const char* error;
	if ((error = dlerror())) {
		printf("%s\n", error);
		return 1;
	}

	return 0;
}
