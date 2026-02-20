#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "feed.h"
#include "users.h"
#include "friends.h"
#include "posts.h"
#include "tree.h"

int return_friend_reposting_statement(tree_node *root, tree_node *tree_root, unsigned int user_id) {
	if (root) {
		if (root != tree_root) {
			if (((repost *)root->data)->user_id == user_id) {
				return 1;
			}
		}

		list_t *children = root->reposts;
		if (!children) {
			return 0;
		}
		list_node_t *head = children->head;

		int partial_response = 0;
		while (head) {
			partial_response = return_friend_reposting_statement(head->data, tree_root, user_id);
			if (partial_response) {
				break;
			}
			head = head->next;
		}

		return partial_response;
	}

	return 0;
}

void compute_friends_reposts(posts_meta **post_meta, friends **friends_network, char *username, unsigned int post_id) {
	post *correct_post = NULL;
	for (unsigned int i = 0; i < (*post_meta)->capacity; i++) {
		if ((*post_meta)->post_list[i]) {
			if ((*post_meta)->post_list[i]->post_id == post_id) {
				correct_post = (*post_meta)->post_list[i];
			}
		}
	}

	tree_node *root = correct_post->recurrent_tree->tree_peaks[0];

	if (!correct_post) {
		return;
	}

	list_t *user_friends = (*friends_network)->graph->adjacency_list[get_user_id(username)];
	if (!user_friends) {
		return;
	}

	list_node_t *head = user_friends->head;
	while (head) {
		if (return_friend_reposting_statement(root, root, get_user_id(((friend_info *)head->data)->friend_name))) {
			printf("%s\n", ((friend_info *)head->data)->friend_name);
		}
		head = head->next;
	}
}

void print_user_reposts(tree_node *root, tree_node *tree_root, char *owner, char *title) {
	if (root) {
		if (root != tree_root) {
			if (((repost *)(root->data))->user_id == get_user_id(owner)) {
				printf("Reposted: \"%s\"\n", title);
			}
		}

		list_t *list = root->reposts;
		if (!list) {
			return;
		}
		list_node_t *head = list->head;

		while (head) {
			print_user_reposts(head->data, tree_root, owner, title);
			head = head->next;
		}
	}
}

void compute_user_profile(posts_meta **post_meta, char *username) {
	for (unsigned int i = 0; i < (*post_meta)->capacity; i++) {
		if ((*post_meta)->post_list[i]) {
			if (!strcmp((*post_meta)->post_list[i]->owner, username)) {
				printf("Posted: \"%s\"\n", (*post_meta)->post_list[i]->title);
			}

			tree_node *root = (*post_meta)->post_list[i]->recurrent_tree->tree_peaks[0];
			char *title = (*post_meta)->post_list[i]->title;

			print_user_reposts(root, root, username, title);
		}
	}
}

void compute_user_feed(posts_meta **post_meta, friends **friends_network, char *username, unsigned int size) {
	unsigned int posts_found = 0;
	unsigned int last_maximum = 10000000;

	while (posts_found < size) {
		unsigned int maximum = 0;
	
		for (unsigned int i = 0; i < (*post_meta)->capacity; i++) {
			if ((*post_meta)->post_list[i]) {
				if ((*post_meta)->post_list[i]->post_id > maximum && (*post_meta)->post_list[i]->post_id < last_maximum) {
					if (is_my_friend(*friends_network, username, (*post_meta)->post_list[i]->owner) ||
									!strcmp(username, (*post_meta)->post_list[i]->owner)) {
						maximum = (*post_meta)->post_list[i]->post_id;
					}
				}
			}
		}

		for (unsigned int i = 0; i < (*post_meta)->capacity; i++) {
			if ((*post_meta)->post_list[i]) {
				if ((*post_meta)->post_list[i]->post_id == maximum) {
					printf("%s: \"%s\"\n", (*post_meta)->post_list[i]->owner, (*post_meta)->post_list[i]->title);
					break;
				}
			}
		}

		if (maximum == last_maximum) {
			break;
		}
		last_maximum = maximum;
		posts_found++;
	}

}

void handle_input_feed(char *input, posts_meta **post_meta, friends **friends_network)
{
	char *commands = strdup(input);
	char *cmd = strtok(commands, "\n ");

	if (!cmd)
		return;

	if (!strcmp(cmd, "feed")) {
		(void)cmd;
		char *name, *feed_size;
		cmd = strtok(NULL, "\n ");
		name = cmd;
		cmd = strtok(NULL, "\n ");
		feed_size = cmd;

		unsigned int feed_size_num = atoi(feed_size);

		compute_user_feed(post_meta, friends_network, name, feed_size_num);
		// TODO: Add function
	} else if (!strcmp(cmd, "view-profile")) {
		(void)cmd;
		char *name;
		cmd = strtok(NULL, "\n ");
		name = cmd;

		compute_user_profile(post_meta, name);
		// TODO: Add function
	} else if (!strcmp(cmd, "friends-repost")) {
		(void)cmd;
		// TODO: Add function
		char *name, *post_id_string;
		cmd = strtok(NULL, "\n ");
		name = cmd;
		cmd = strtok(NULL, "\n ");
		post_id_string = cmd;

		unsigned int post_id = atoi(post_id_string);

		compute_friends_reposts(post_meta, friends_network, name, post_id);
	}
	else if (!strcmp(cmd, "common-groups"))
		(void)cmd;
		// TODO: Add function

	free(commands);
}
