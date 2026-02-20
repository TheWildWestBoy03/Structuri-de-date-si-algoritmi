/**
 * The entrypoint of the homework. Every initialization must be done here
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "users.h"
#include "friends.h"
#include "posts.h"
#include "feed.h"

/**
 * Initializez every task based on which task we are running
*/
void init_tasks(void)
{
	#ifdef TASK_1

	#endif

	#ifdef TASK_2

	#endif

	#ifdef TASK_3

	#endif
}

/**
 * Entrypoint of the program, compiled with different defines for each task
*/
int main(void)
{
	init_users();

	init_tasks();

	char *input = (char *)malloc(MAX_COMMAND_LEN);
	friends *friends_network = NULL;
	posts_meta *posts_meta = NULL;

	while (1) {
		char *command = fgets(input, MAX_COMMAND_LEN, stdin);

		// If fgets returns null, we reached EOF
		if (!command)
			break;

		#ifdef TASK_1
		handle_input_friends(input, &friends_network);
		#endif

		#ifdef TASK_2
		(void)friends_network;
		handle_input_posts(input, &posts_meta);
		#endif

		#ifdef TASK_3
		(void)friends_network;
		handle_input_feed(input, &posts_meta, &friends_network);
		#endif
	}

	free_users();
	free(input);

	if (friends_network) {
		free_graph(&(friends_network->graph));
		friends_network->graph = NULL;
		free(friends_network);
		friends_network = NULL;
	}

	if (posts_meta) {
		// free the structures used for the second task
		#ifdef TASK_2
		free_structures(&posts_meta);
		#endif
	}
	return 0;
}
