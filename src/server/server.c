#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>

#include <getopt.h>

#include "loader.h"
#include "board.h"
#include "string.h"
#include "movef.h"

#include "time.h"

#define UNUSED(x)  (void)(x)
#define MAX_ROUNDS 10000

unsigned int m = 9;
char t = 'c';

/**
 * \brief Check if the player player_id has won
 *
 * \param b The board
 * \param player_id The player color
 *
 * \return 1 if the player has won, 0 otherwise
 */
int player_has_won(struct board_t* b, enum color_t player_id)
{
	return graph__is_start(b->graph, b->pos[player_id], (player_id == BLACK) ? WHITE : BLACK);
}

/**
 * \brief Store the values of arguments passed in the command
 *
 * \param argc The number of arguments
 * \param argv The arguments
 */
void opt_process(int argc, char* argv[])
{
	time_t seed = time(NULL);
	printf("Seed : %ld\n", seed);
	srand(seed);

	if (argc < 3) {
		fprintf(stderr, "Usage: %s [-m <largeur du plateau>] [-t <type : c,t,h,s>] client1.so client2.so\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	int c;
	while ((c = getopt(argc, argv, "m:t:")) != -1) {
		switch (c) {
			case 'm':
				m = atoi(optarg);
				break;
			case 't':
				t = optarg[0];
				if (t != 'c' && t != 't' && t != 'h' && t != 's') {
					fprintf(stderr, "Usage: %s [-m <largeur du plateau>] [-t <type : c,t,h,s>] client1.so client2.so\n", argv[0]);
					exit(EXIT_FAILURE);
				}
				break;
			default:
				fprintf(stderr, "Usage: %s [-m <largeur du plateau>] [-t <type : c,t,h,s>] client1.so client2.so\n", argv[0]);
				exit(EXIT_FAILURE);
		}
	}

	switch (t) {
		case 't':
		case 'h':
			if (m % 3 != 0) {
				fprintf(stderr, "m should multiple of 3 with h == 't'\n");
				exit(EXIT_FAILURE);
			}
			break;
		case 's':
			if (m % 5 != 0) {
				fprintf(stderr, "m should multiple of 5 with h == 's'\n");
				exit(EXIT_FAILURE);
			}
			break;
	}
}

int main(int argc, char* argv[])
{
	opt_process(argc, argv);
	int ret_code = 0;

	struct client client1, client2;
	int error_code;
	if ((error_code = load_client(argv[argc - 2], &client1))) {
		printf("Unable to load client %s. Error code : %u\n", argv[argc - 2], error_code);
		return 1;
	}

	if ((error_code = load_client(argv[argc - 1], &client2))) {
		printf("Unable to load client %s. Error code : %u\n", argv[argc - 1], error_code);
		ret_code = 1;
		goto destroy_client1;
	}

	const char* name_client1 = client1.get_player_name();
	const char* name_client2 = client2.get_player_name();
	printf("Player 1 name : %s\n", name_client1);
	printf("Player 2 name : %s\n", name_client2);

	//////////////////////////////////////////
	struct board_t b;
	switch (t) {
		case 'c':
			board__init(&b, graph__square(m));
			break;
		case 't':
			board__init(&b, graph__toric(m));
			break;
		case 'h':
			board__init(&b, graph__h_shaped(m));
			break;
		case 's':
			board__init(&b, graph__s_shaped(m));
			break;
		default:
			break;
	}

	struct graph_t* g1 = graph__copy(b.graph);
	struct graph_t* g2 = graph__copy(b.graph);

	client1.initialize(BLACK, g1, b.num_walls[0]);
	client2.initialize(WHITE, g2, b.num_walls[1]);

	struct move_t last_move = client1.play((struct move_t) {0, {no_edge(), no_edge()}, NO_TYPE, NO_COLOR});
	if (last_move.t != MOVE || !board__apply_move(&b, &last_move)) {
		printf("Error with the first move of player 1.\n");
		ret_code = 1;
		goto destroy;
	}
	printf("%s's starting position : %lu\n", name_client1, last_move.m);

	last_move = client2.play(last_move);
	if (last_move.t != MOVE || !board__apply_move(&b, &last_move)) {
		printf("Error with the first move of player 2.\n");
		ret_code = 1;
		goto destroy;
	}
	printf("%s's starting position : %lu\n", name_client2, last_move.m);

	unsigned int nb_round_with_move = 0;
	unsigned int nb_round = 1;
	while (nb_round_with_move < (20 * m)) {
		last_move = client1.play(last_move);
		print_move(&last_move, name_client1, nb_round);
		if (last_move.t == NO_TYPE || !board__apply_move(&b, &last_move)) {
			printf("%s didn't play well.\n", name_client1);
			goto destroy;
		}
		if (last_move.t != MOVE)
			nb_round_with_move = 0;
		if (player_has_won(&b, BLACK)) {
			printf("%s won.\n", name_client1);
			goto destroy;
		}
		last_move = client2.play(last_move);
		print_move(&last_move, name_client2, nb_round);
		if (last_move.t == NO_TYPE || !board__apply_move(&b, &last_move)) {
			printf("%s didn't play well.\n", name_client2);
			break;
		}
		if (last_move.t != MOVE)
			nb_round_with_move = 0;
		if (player_has_won(&b, WHITE)) {
			printf("%s won.\n", name_client2);
			break;
		}
		nb_round_with_move++;
		nb_round++;
	}
	if (nb_round_with_move >= (20 * m))
		printf("Too much rounds with moves\n");

destroy:
	client1.finalize();
	client2.finalize();

	board__release(&b);
	/////////////////////////////////////////

	if ((error_code = unload_client(&client2))) {
		printf("Unable to unload client %s. Error code : %u\n", argv[argc - 1], error_code);
		return 1;
	}

destroy_client1:
	if ((error_code = unload_client(&client1))) {
		printf("Unable to unload client %s. Error code : %u\n", argv[argc - 2], error_code);
		return 1;
	}

	return ret_code;
}
