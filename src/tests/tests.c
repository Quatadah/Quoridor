#include <dlfcn.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>

#include "loader.h"
#include "tests.h"
#include "clients/client.h"
#include "graphfunc.h"
#include "boardfunc.h"
#include "test_move.h"

#define TESTCASE(msg, f) printf("%s : %s\033[0m\n", msg, f() ? "\033[0;32mPASSED" : "\033[0;31mFAILED")
#define TESTCASE_ARGS(msg, f, ...) \
	printf("%s " #__VA_ARGS__ " : %s\033[0m\n", msg, f(__VA_ARGS__) ? "\033[0;32mPASSED" : "\033[0;31mFAILED")

void test_graph()
{
	TESTCASE("graph__init", test_graph__init);
	TESTCASE("graph__copy", test_graph__copy);
	TESTCASE("graph__set_edge", test_graph__set_edge);
	TESTCASE("graph__get_edge", test_graph__get_edge);
	TESTCASE("graph__square", test_graph__square);
	TESTCASE("graph__toric", test_graph__toric);
	TESTCASE("graph__s_shaped", test_graph__s_shaped);
	TESTCASE("graph__h_shaped", test_graph__h_shaped);
	TESTCASE("graph__is_start", test_graph__is_start);
	TESTCASE("graph__claim_node", test_graph__claim_node);
	TESTCASE("graph__unclaim_node", test_graph__unclaim_node);
}

void test_board()
{
	TESTCASE("board__apply_move no_type", test_board__apply_move_no_type);
	TESTCASE("board__apply_move wall", test_board__apply_move_wall);
	TESTCASE("board__apply_move move", test_board__apply_move_move);
	TESTCASE("board__check_move no_type", test_board__check_move_no_type);
	TESTCASE("board__check_move wall", test_board__check_move_wall);
	TESTCASE("board__check_move move", test_board__check_move_move);
	TESTCASE("board__init", test_board__init);
	TESTCASE("board__are_equal", test_board__are_equal);
}

void test_move()
{
	TESTCASE("no_edge", test_no_edge);
	TESTCASE("is_no_edge", test_is_no_edge);
}

void test_client(const char* name)
{
	printf("Tests for client %s\n", name);

	struct client client;
	int error_code;
	if ((error_code = load_client(name, &client))) {
		printf("Unable to load client %s. Error code : %u\n", name, error_code);
		exit(1);
	}

	TESTCASE_ARGS("  initialize", test_initialize, &client);
	TESTCASE_ARGS("  get_player_name", test_get_player_name, &client);
	TESTCASE_ARGS("  test_play", test_play, &client);

	if ((error_code = unload_client(&client))) {
		printf("Unable to unload client %s. Error code : %u\n", name, error_code);
		exit(2);
	}
}

int main(int argc, const char* argv[])
{
	DIR* d;
	struct dirent* dir;
	const char* prefix = "install";
	if (argc > 1)
		prefix = argv[1];

	d = opendir(prefix);

	if (!d) {
		d = opendir(".");
		prefix = ".";
	}
	if (d) {
		while ((dir = readdir(d)) != NULL) {
			unsigned int l = strlen(dir->d_name);
			if (l > 3 && dir->d_name[l - 3] == '.' && dir->d_name[l - 2] == 's' && dir->d_name[l - 1] == 'o') {
				char name[258];
				sprintf(name, "%s/%s", prefix, dir->d_name);
				test_client(name);
			}
		}
		closedir(d);
	}

	test_graph();
	test_board();
	test_move();

	return 0;
}
