#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "users.h"
#include "posts.h"
#include "tree.h"
#include <sys/types.h>

void compute_common_repost(posts_meta **post_meta, unsigned int post_id, unsigned int first_id, unsigned int second_id) {
	for (unsigned int i = 0; i < (*post_meta)->capacity; i++) {
		if (!(*post_meta)->post_list[i]) {
			return;
		}

		if ((*post_meta)->post_list[i]->post_id == post_id) {
			tree_node *root = (*post_meta)->post_list[i]->recurrent_tree->tree_peaks[0];
			tree_node *common_request = compute_lca(root, offsetof(repost, repost_id), sizeof(unsigned int), &first_id, &second_id);

			if (common_request) {
				if (!((repost *)(common_request->data))->repost_id) {
					((repost *)(common_request->data))->repost_id = post_id;
				} 
				printf("The first common repost of %d and %d is %d\n", first_id, second_id, ((repost *)(common_request->data))->repost_id);
			}
			break;
		}
	}
}

void like(posts_meta **posts_meta, char *name, unsigned int post_id, unsigned int repost_id) {
	unsigned int user_id = get_user_id(name);

	for (unsigned int i = 0; i < (*posts_meta)->capacity; i++) {
		if (!(*posts_meta)->post_list[i]) {
			return;
		}

		if ((*posts_meta)->post_list[i]->post_id == post_id) {
			if (repost_id) {
				// we have to search the correct post/repost here
				tree_node *root = (*posts_meta)->post_list[i]->recurrent_tree->tree_peaks[0];
				tree_node *parent_node = retrieve_node_via_key(0, offsetof(repost, repost_id), sizeof(unsigned int), &repost_id, root);

				if (!parent_node) {
					return;
				}

				if (!(*posts_meta)->appreciations_matrix[user_id][post_id]) {
					((repost *)parent_node->data)->number_of_likes++;
					(*posts_meta)->appreciations_matrix[user_id][post_id] = 1;
					printf("User %s liked repost \"%s\"\n", name, (*posts_meta)->post_list[i]->title);
				} else {
					((repost *)parent_node->data)->number_of_likes--;
					(*posts_meta)->appreciations_matrix[user_id][post_id] = 0;
					printf("User %s unliked repost \"%s\"\n", name, (*posts_meta)->post_list[i]->title);
				}

			} else {
				if (!(*posts_meta)->appreciations_matrix[user_id][post_id]) {
					(*posts_meta)->post_list[i]->number_of_likes++;
					(*posts_meta)->appreciations_matrix[user_id][post_id] = 1;
					printf("User %s liked post \"%s\"\n", name, (*posts_meta)->post_list[i]->title);

				} else {
					(*posts_meta)->post_list[i]->number_of_likes--;
					(*posts_meta)->appreciations_matrix[user_id][post_id] = 0;
					printf("User %s unliked post \"%s\"\n", name, (*posts_meta)->post_list[i]->title);
				}
			}
		}
	}
}

void compute_ratio(posts_meta **posts_meta, unsigned int post_id) {
	for (unsigned int i = 0; i < (*posts_meta)->capacity; i++) {
		if (!(*posts_meta)->post_list[i]) {
			return;
		}

		if ((*posts_meta)->post_list[i]->post_id == post_id) {
			unsigned int number_of_likes = (*posts_meta)->post_list[i]->number_of_likes;
			tree *current_tree = (*posts_meta)->post_list[i]->recurrent_tree;
			if (!current_tree) {
				return;
			}
	
			tree_node *root = current_tree->tree_peaks[0];
			tree_node *better_repost_node = retrieve_node_via_key(1, offsetof(repost, number_of_likes), sizeof(unsigned int), &number_of_likes, root);
	
			if (!better_repost_node) {
				printf("The original post is the highest rated\n");
			} else {
				printf("Post %d got ratio'd by repost %d\n", post_id, ((repost *)better_repost_node->data)->repost_id);
			}
			
			return;
		}
	}
}

void delete_entity(posts_meta **post_meta, unsigned int post_id, unsigned int repost_id) {
	for (unsigned int i = 0; i < (*post_meta)->capacity; i++) {
		if (!(*post_meta)->post_list[i]) {
			return;
		}

		if ((*post_meta)->post_list[i]->post_id == post_id) {
			if (repost_id) {
				tree_node *root = retrieve_node_via_key(0, offsetof(repost, repost_id), sizeof(unsigned int), &repost_id,
						(*post_meta)->post_list[i]->recurrent_tree->tree_peaks[0]);

				tree_node *parent = retrieve_node_parent_via_key(offsetof(repost, repost_id), sizeof(unsigned int), &repost_id,
						(*post_meta)->post_list[i]->recurrent_tree->tree_peaks[0]);
						
				tree_node *old_root_address = root;
				(void)old_root_address;
				// printf("parent: %p\n", parent);
				// printf("root: %p\n", root);
				delete_tree(&root, &root);

				if (parent) {
					list_t *children = parent->reposts;
					list_node_t *head = children->head;
					// printf("list address before: %p\n", children);
	
					while (head) {
						if (old_root_address == head->data) {
							if (!head->next && !head->prev) {
								free(parent->reposts->head);
								parent->reposts->head = NULL;
								parent->reposts->tail = NULL;

								free_list(0, &parent->reposts);
								parent->reposts = NULL;
								children = NULL;

								break;
							}
	
							if (head->prev) {
								head->prev = head->prev->next;
							} else {
								children->head = children->head->next;
								children->head->prev = NULL;
							}
							if (head->next) {
								head->next = head->next->prev;
							} else {
								children->tail = children->tail->prev;
								children->tail->next = NULL;
							}

							free(head);
							children->head = NULL;
							children->tail = NULL;
							
							break;
						}
	
						head = head->next;
					}

					if (children) {
						children->size--;
					}

					printf("Deleted repost #%d of post \"%s\"\n", repost_id, (*post_meta)->post_list[i]->title);
				}
			} else {
				delete_tree(&(*post_meta)->post_list[i]->recurrent_tree->tree_peaks[0],
								&(*post_meta)->post_list[i]->recurrent_tree->tree_peaks[0]);

				free((*post_meta)->post_list[i]->recurrent_tree->tree_peaks);
				(*post_meta)->post_list[i]->recurrent_tree->tree_peaks = NULL;
				free((*post_meta)->post_list[i]->recurrent_tree);
				(*post_meta)->post_list[i]->recurrent_tree = NULL;
				free((*post_meta)->post_list[i]);
				(*post_meta)->post_list[i] = NULL;
				printf("Deleted \"%s\"\n", (*post_meta)->post_list[i]->title);
			}

		}
	}
}

void get_likes(posts_meta **posts_meta, unsigned int post_id, unsigned int repost_id) {
	for (unsigned int i = 0; i < (*posts_meta)->capacity; i++) {
		if (!(*posts_meta)->post_list[i]) {
			return;
		}

		if ((*posts_meta)->post_list[i]->post_id == post_id) {
			if (repost_id) {
				tree *current_tree = (*posts_meta)->post_list[i]->recurrent_tree;
				if (!current_tree) {
					return;
				}
		
				tree_node *root = current_tree->tree_peaks[0];
				tree_node *better_repost_node = retrieve_node_via_key(0, offsetof(repost, repost_id),
						 sizeof(unsigned int), &repost_id, root);
	
				if (better_repost_node) {
					printf("Repost #%d has %d likes\n", ((repost *)better_repost_node->data)->repost_id,
							 ((repost *)better_repost_node->data)->number_of_likes);
				}
			} else {
				printf("Post \"%s\" has %d likes\n", (*posts_meta)->post_list[i]->title,
							(*posts_meta)->post_list[i]->number_of_likes);
			}

			break;
		}
	}
}

void print_reposts(tree_node *root, tree_node *tree_root) {
	if (root) {
		if (root != tree_root) {
			if (root->data) {
				if (((repost *)(root->data))->repost_id) {
					printf("Repost #%d by %s\n", ((repost *)(root->data))->repost_id, get_user_name(((repost *)(root->data))->user_id));
				}
			}
		}
		
		list_t *list = root->reposts;
		if (!list) {
			return;
		}

		list_node_t *list_head = list->head;
		if (!list_head) {
			return;
		}

		while (list_head) {
			print_reposts(list_head->data, tree_root);
			list_head = list_head->next;
		}
	}
}

void get_reposts(posts_meta **posts_meta, unsigned int post_id, unsigned int repost_id) {
	for (unsigned int i = 0; i < (*posts_meta)->capacity; i++) {
		if (!(*posts_meta)->post_list[i]) {
			return;
		}

		if ((*posts_meta)->post_list[i]->post_id == post_id) {
			if (repost_id) {
				tree_node *root = retrieve_node_via_key(0, offsetof(repost, repost_id), sizeof(unsigned int), &repost_id,
						 (*posts_meta)->post_list[i]->recurrent_tree->tree_peaks[0]);

				print_reposts(root, NULL);
			} else {
				printf("\"%s\" - Post by %s\n",(*posts_meta)->post_list[i]->title, get_user_name((*posts_meta)->post_list[i]->user_id));
				print_reposts((*posts_meta)->post_list[i]->recurrent_tree->tree_peaks[0], (*posts_meta)->post_list[i]->recurrent_tree->tree_peaks[0]);
			}

			break;
		}
	}
}

repost *allocate_new_repost(unsigned int repost_id, unsigned int user_id) {
	repost *new_repost = calloc(1, sizeof(repost));

	new_repost->number_of_likes = 0;
	new_repost->repost_id = repost_id;
	new_repost->repost_timeprint = 0;
	new_repost->user_id = user_id;

	return new_repost;
}

void free_structures(posts_meta **post_meta) {
	for (unsigned int i = 0; i < (*post_meta)->capacity; i++) {
		if ((*post_meta)->post_list[i]) {
			tree_node *root = (*post_meta)->post_list[i]->recurrent_tree->tree_peaks[0];
			free(root->data);
			root->data = NULL;
			delete_tree(&root, &root);
			root = NULL;
			
			free((*post_meta)->post_list[i]->recurrent_tree->tree_peaks);
			tree *post_tree = (*post_meta)->post_list[i]->recurrent_tree;
			free(post_tree);
			post_tree = NULL;
	
			free((*post_meta)->post_list[i]->owner);
			(*post_meta)->post_list[i]->owner = NULL;
			free((*post_meta)->post_list[i]->title);
			(*post_meta)->post_list[i]->title = NULL;

			free((*post_meta)->post_list[i]);
			(*post_meta)->post_list[i] = NULL;
		}
	}

	for (unsigned int i = 0; i < (*post_meta)->maximum_users; i++) {
		free((*post_meta)->appreciations_matrix[i]);
		(*post_meta)->appreciations_matrix[i] = NULL;
	}

	free((*post_meta)->appreciations_matrix);
	(*post_meta)->appreciations_matrix = NULL;

	free((*post_meta)->post_list);
	(*post_meta)->post_list = NULL;

	free(*post_meta);
	*post_meta = NULL;
}

void create_repost(posts_meta **posts_meta, char *owner, unsigned int parent_post_id, unsigned int parent_repost_id) {
	for (unsigned int i = 0; i < (*posts_meta)->capacity; i++) {
		if (!(*posts_meta)->post_list[i]) {
			return;
		}

		if ((*posts_meta)->post_list[i]->post_id == parent_post_id) {
			tree *post_tree = (*posts_meta)->post_list[i]->recurrent_tree;

			// create a new tree node repost in the parent post node
			tree_node *post_tree_root = (*posts_meta)->post_list[i]->recurrent_tree->tree_peaks[0];

			tree_node *new_tree_node = create_node(NULL, post_tree->datasize);
			new_tree_node->data = malloc(sizeof(repost));
			((repost *)new_tree_node->data)->repost_id = (*posts_meta)->pending_post_id;
			((repost *)new_tree_node->data)->user_id = get_user_id(owner);
			((repost *)new_tree_node->data)->number_of_likes = 0;

			(*posts_meta)->pending_post_id++;

			if (parent_repost_id == 0) {
				// add the repost linked to the post root
				if (post_tree_root->reposts == NULL) {
					create_list(&post_tree_root->reposts, 0, 0, 0, 0, sizeof(tree_node), 0, NULL);
				}

				add_node_to_back(0, &post_tree_root->reposts, new_tree_node);
				post_tree_root->reposts->size++;

			} else {
				// move recursively to the bottom of the reposts tree
				tree_node *parent_node = retrieve_node_via_key(0, offsetof(repost, repost_id), sizeof(unsigned int), &parent_repost_id, post_tree_root);

				if (!parent_node) {
					return;
				}

				if (!parent_node->reposts) {
					create_list(&parent_node->reposts, 0, 0, 0, 0, sizeof(tree_node), 0, NULL);
				}
				add_node_to_back(0, &parent_node->reposts, new_tree_node);
				post_tree_root->reposts->size++;
			}

			
			printf("Created repost #%d for %s\n", ((repost *)(new_tree_node->data))->repost_id, owner);
			free(new_tree_node);

			break;
		}
	}


}

void create_post_element(posts_meta **post_meta, post **new_post, char *owner, char *title) {
	(*new_post) = calloc(1, sizeof(post));

	(void)post_meta;
	(void)owner;
	(void)title;

	(*new_post)->owner = calloc(MAX_COMMAND_LEN, 1);
	strcpy((*new_post)->owner, owner);
	(*new_post)->title = calloc(MAX_TITLE_LENGTH, 1);
	strcpy((*new_post)->title, title);
	(*new_post)->user_id = get_user_id(owner);
	(*new_post)->post_id = (*post_meta)->pending_post_id;
	(*new_post)->pending_repost_id = 1;

	(*post_meta)->pending_post_id++;
	(*post_meta)->size++;
}

void create_post(posts_meta **post_meta, char *owner, char *title) {
	for (unsigned int i = 0; i < (*post_meta)->capacity; i++) {
		if (!(*post_meta)->post_list[i]) {
			create_post_element(post_meta, &(*post_meta)->post_list[i], owner, title);
			create_tree(&(*post_meta)->post_list[i]->recurrent_tree, sizeof(repost), sizeof(tree), MAX_POSTS);
			break;
		}
	}

	printf("Created \"%s\" for %s\n", title, owner);
}

void initialize_posts_array(posts_meta **post_meta, unsigned int maxsize, unsigned int maximum_users) {
	(void)maximum_users;
	(void)maxsize;
	(void)post_meta;

	(*post_meta) = calloc(1, sizeof(posts_meta));
	(*post_meta)->size = 0;
	(*post_meta)->maximum_users = maximum_users;
	(*post_meta)->pending_post_id = 1;

 	(*post_meta)->post_list = malloc(maxsize * sizeof(post *));
	(*post_meta)->capacity = maxsize;

	(*post_meta)->appreciations_matrix = NULL;
	if (!(*post_meta)->post_list) {
		return;
	}
	
	for (unsigned int i = 0; i < maxsize; i++) {
		(*post_meta)->post_list[i] = NULL;
	}

	initialize_liking_user_post_matrix(&(*post_meta)->appreciations_matrix, (*post_meta)->maximum_users);
}
 
void initialize_liking_user_post_matrix(int ***matrix, unsigned int maximum_users) {
	(*matrix) = calloc(maximum_users, sizeof(int *));
	
	for (unsigned int i = 0; i < maximum_users; i++) {
		(*matrix)[i] = calloc(MAX_POSTS, sizeof(int));
	}
}

u_int16_t retrieve_num_of_users(u_int16_t id) {
	while (!get_user_name(id)) {
		id--;
	}
	return id;
}

void handle_input_posts(char *input, posts_meta **post_meta)
{
	char *commands = strdup(input);
	char *cmd = strtok(commands, "\n ");

	if (!cmd)
		return;

	u_int16_t network_size = 1000;

	if (!(*post_meta)) {
		initialize_posts_array(post_meta, MAX_POSTS, retrieve_num_of_users(network_size) + 1);
	}

	if (!strcmp(cmd, "create")) {
		// TODO: Add function
		char *owner, *title;
		cmd = strtok(NULL, "\n ");
		owner = cmd;
		cmd = strtok(NULL, "\"");
		title = cmd;

		if (owner && title) {
			create_post(post_meta, owner, title);
		}
	}
	else if (!strcmp(cmd, "repost")) {
		// TODO: Add function

		char *owner, *post_id_string, *repost_id_string;
		unsigned int post_id, repost_id;
		(void) post_id;
		(void) repost_id;

		cmd = strtok(NULL, "\n ");
		owner = cmd;

		cmd = strtok(NULL, "\n ");
		post_id_string = cmd;
		post_id = atoi(post_id_string);

		cmd = strtok(NULL, "\n ");
		repost_id_string = cmd;

		if (repost_id_string) {
			repost_id = atoi(repost_id_string);
		} else {
			repost_id = 0;
		}

		if (owner) {
			create_repost(post_meta, owner, post_id, repost_id);
		}
	} else if (!strcmp(cmd, "common-repost")) {
		char *first_key, *second_key, *post_id_string;
		cmd = strtok(NULL, "\n ");
		post_id_string = cmd;
		cmd = strtok(NULL, "\n ");
		first_key = cmd;
		cmd = strtok(NULL, "\n ");
		second_key = cmd;

		unsigned int post_id = atoi(post_id_string);
		unsigned int first_repost_id = atoi(first_key), second_repost_id = atoi(second_key);

		compute_common_repost(post_meta, post_id, first_repost_id, second_repost_id);
		// TODO: Add function
		
	} else if (!strcmp(cmd, "like")) {
		// TODO: Add function
		char *user, *post_id_string, *repost_id_string;
		unsigned int post_id, repost_id;
		(void) post_id;
		(void) repost_id;

		cmd = strtok(NULL, "\n ");
		user = cmd;

		cmd = strtok(NULL, "\n ");
		post_id_string = cmd;
		post_id = atoi(post_id_string);

		cmd = strtok(NULL, "\n ");
		repost_id_string = cmd;

		if (repost_id_string) {
			repost_id = atoi(repost_id_string);
		} else {
			repost_id = 0;
		}

		if (user) {
			like(post_meta, user, post_id, repost_id);
		}
	}
	else if (!strcmp(cmd, "ratio")) {
		// TODO: Add function
		char *post_id_string;
		unsigned int post_id;
		(void) post_id;

		cmd = strtok(NULL, "\n ");
		post_id_string = cmd;

		post_id = atoi(post_id_string);

		compute_ratio(post_meta, post_id);
	}
	else if (!strcmp(cmd, "delete")) {
		char *post_id_string, *repost_id_string;
		unsigned int post_id, repost_id;
		(void) repost_id;
		(void) post_id;

		cmd = strtok(NULL, "\n ");
		post_id_string = cmd;
		post_id = atoi(post_id_string);

		cmd = strtok(NULL, "\n ");
		repost_id_string = cmd;

		if (repost_id_string) {
			repost_id = atoi(repost_id_string);
		} else {
			repost_id = 0;
		}

		delete_entity(post_meta, post_id, repost_id);
		// TODO: Add function
	} else if (!strcmp(cmd, "get-likes")) {
		// TODO: Add function

		char *post_id_string, *repost_id_string;
		unsigned int post_id, repost_id;
		(void) repost_id;
		(void) post_id;

		cmd = strtok(NULL, "\n ");
		post_id_string = cmd;
		post_id = atoi(post_id_string);

		cmd = strtok(NULL, "\n ");
		repost_id_string = cmd;

		if (repost_id_string) {
			repost_id = atoi(repost_id_string);
		} else {
			repost_id = 0;
		}

		get_likes(post_meta, post_id, repost_id);
	} else if (!strcmp(cmd, "get-reposts")) {
		// TODO: Add function
		char *post_id_string, *repost_id_string;
		unsigned int post_id, repost_id;

		cmd = strtok(NULL, "\n ");
		post_id_string = cmd;
		post_id = atoi(post_id_string);

		cmd = strtok(NULL, "\n ");
		repost_id_string = cmd;

		if (repost_id_string) {
			repost_id = atoi(repost_id_string);
		} else {
			repost_id = 0;
		}

		get_reposts(post_meta, post_id, repost_id);

	} else if (!strcmp(cmd, "get-likes")) {
		// TODO: Add function

		char *post_id_string, *repost_id_string;
		unsigned int post_id, repost_id;
		(void) repost_id;
		(void) post_id;
		cmd = strtok(NULL, "\n ");
		post_id_string = cmd;
		post_id = atoi(post_id_string);

		cmd = strtok(NULL, "\n ");
		repost_id_string = cmd;

		if (repost_id_string) {
			repost_id = atoi(repost_id_string);
		} else {
			repost_id = 0;
		}

		get_likes(post_meta, post_id, repost_id);
	}

	free(commands);
}

