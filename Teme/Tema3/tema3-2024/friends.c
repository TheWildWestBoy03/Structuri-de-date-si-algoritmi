#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#include "friends.h"
#include "users.h"
#include <sys/types.h>

void* is_my_friend(friends *friends, void *key1, void *key2) {
	list_t *user_friends = friends->graph->adjacency_list[get_user_id(key1)];
	list_node_t *head = user_friends->head;

	while (head) {
		if (!strcmp(((friend_info *)(head->data))->friend_name, (char *)key2)) {
			return head->data;
		}

		head = head->next;
	}

	return NULL;
}


void compute_distance(friends *friends, void *key1, void *key2) {
	for (unsigned int i = 0; i < friends->graph->size; i++) {
		friends->graph->visited[i] = 0;
		friends->graph->distances[i] = INT_LEAST32_MAX - 1;
	}

	// create the queue, necessary for BFS search
	list_t *list_queue = NULL;
	create_list(&list_queue, sizeof(unsigned int), MAX_COMMAND_LEN, offsetof(friend_info, friend_id), offsetof(friend_info, friend_name), sizeof(friend_info), MAX_COMMAND_LEN, NULL);

	friend_info *friend_infos = calloc(1, sizeof(friend_info));
	friend_infos->friend_id = get_user_id((char *)key1);
	friend_infos->friend_name = calloc(MAX_COMMAND_LEN, 1);
	strcpy(friend_infos->friend_name, (char *)key1);

	list_queue->size++;
	add_node_to_back(2, &list_queue, friend_infos);

	free(friend_infos->friend_name);
	friend_infos->friend_name = NULL;
	free(friend_infos);
	friend_infos = NULL;

	char *current_key = calloc(MAX_COMMAND_LEN, 1);
	friends->graph->visited[get_user_id(key1)] = 1;
	friends->graph->distances[get_user_id(key1)] = 0;

	while (list_queue->size) {
		if (!list_queue->head) {
			break;
		}

		if (!list_queue->head->data) {
			break;
		}

		if (!((friend_info *) list_queue->head->data)->friend_name) {
			break;
		}

		strcpy(current_key, ((friend_info *) list_queue->head->data)->friend_name);
		remove_node_from_front(2, &list_queue);
		list_queue->size--;
		list_t *current_list_to_check = friends->graph->adjacency_list[get_user_id(current_key)];
		list_node_t *head = current_list_to_check->head;

		while (head) {
			char *current_name = ((friend_info *)(head->data))->friend_name;

			if (!friends->graph->visited[get_user_id(current_name)]) {
				friend_info *new_info = calloc(1, sizeof(friend_info));
				
				friends->graph->visited[get_user_id(current_name)] = 1;
				memcpy(new_info, (friend_info *)head->data, sizeof(friend_info));
				new_info->friend_name = calloc(MAX_COMMAND_LEN, 1);
				strcpy(new_info->friend_name, current_name);
				
				add_node_to_back(2, &list_queue, new_info);
				list_queue->size++;
				
				free(new_info->friend_name);
				new_info->friend_name = NULL;
				free(new_info);
				new_info = NULL;
				
				unsigned int index_current = get_user_id(current_name);
				unsigned int index_neighbor = get_user_id(current_key);
				
				friends->graph->distances[index_current] = friends->graph->distances[index_neighbor] + 1;
			}

			head = head->next;
		}
	}

	free(current_key);
	unsigned int index_current = get_user_id(key2);
	if (friends->graph->distances[index_current] == INT_LEAST32_MAX - 1) {
		printf("There is no way to get from %s to %s\n", (char *)key1, (char *)key2);
	} else {
		printf("The distance between %s - %s is %d\n", (char *)key1, (char *)key2, friends->graph->distances[index_current]);
	}

	free_list(2, &list_queue);
}

void compute_common_friends(friends *friends, void *key1, void *key2) {
	unsigned int number_of_common_friends = 0;

	list_t *first_friend_list = friends->graph->adjacency_list[get_user_id(key1)];
	list_t *second_friend_list = friends->graph->adjacency_list[get_user_id(key2)];

	(void)number_of_common_friends;
	(void)first_friend_list;
	(void)second_friend_list;

	list_t *sorted_common_friends;
	create_list(&sorted_common_friends, sizeof(unsigned int), MAX_COMMAND_LEN, offsetof(friend_info, friend_id), offsetof(friend_info, friend_name), sizeof(friend_info), MAX_COMMAND_LEN, NULL);
	sorted_common_friends->size = 0;

	list_node_t *first_list_head = first_friend_list->head;
	
	while (first_list_head) {
		char *first_list_friend_name = ((friend_info *)first_list_head->data)->friend_name;		
		list_node_t *second_list_head = second_friend_list->head;
		
		while (second_list_head) {
			char *second_list_friend_name = ((friend_info *)second_list_head->data)->friend_name;		

			if (!strcmp(first_list_friend_name, second_list_friend_name)) {

				// creating the new node to be inserted in the sorted common names linked list
				list_node_t *new_node = calloc(1, sizeof(list_node_t));
				new_node->next = new_node->prev = NULL;
				new_node->data = calloc(1, sorted_common_friends->datasize);
				((friend_info *) new_node->data)->friend_name = calloc(MAX_COMMAND_LEN, 1);
				strcpy(((friend_info *) new_node->data)->friend_name, second_list_friend_name);
				((friend_info *) new_node->data)->friend_id = get_user_id(((friend_info *) new_node->data)->friend_name);
				
				if (sorted_common_friends->head == NULL) {
					sorted_common_friends->head = new_node;
					sorted_common_friends->tail = new_node;
				} else {
					list_node_t *head = sorted_common_friends->head;
					list_node_t *tail = NULL;

					while (head) {
						unsigned int list_node_friend_id = (((friend_info *)head->data)->friend_id);
						unsigned int new_node_friend_id = get_user_id(second_list_friend_name);

						if (list_node_friend_id >= new_node_friend_id) {
							break;
						}

						tail = head;
						head = head->next;
					}

					if (head) {
						new_node->next = head;
						new_node->prev = head->prev;
						
						if (head->prev) {
							head->prev->next = new_node;
						} else {
							new_node->next = sorted_common_friends->head;
							sorted_common_friends->head->prev = new_node;
							sorted_common_friends->head = new_node;
						}

						head->prev = new_node;
					} else {
						tail->next = new_node;
						new_node->prev = tail;
						new_node->next = NULL;
						tail = new_node;
					}
				}

				sorted_common_friends->size++;
			}
			second_list_head = second_list_head->next;
		}

		first_list_head = first_list_head->next;
	}

	if (sorted_common_friends->size) {
		printf("The common friends between %s and %s are:\n", (char *)key1, (char *)key2);

		list_node_t *head = sorted_common_friends->head;
		while(head) {
			friend_info *curr_friend_info = head->data;
			printf("%s\n", curr_friend_info->friend_name);
			head = head->next;
		}
	} else {
		printf("No common friends for %s and %s\n", (char *)key1, (char *)key2);
	}

	free_list(2, &sorted_common_friends);
}

void display_friends(friends *friends, void *key1) {
	printf("%s has %d friends\n", (char *)key1, friends->graph->adjacency_list[get_user_id(key1)]->size);
}

void compute_popular(friends *friends, void *key1) {
	list_t *user_friends = friends->graph->adjacency_list[get_user_id(key1)];
	list_node_t *user_friends_head = user_friends->head;

	unsigned int maximum_friends = 0;
	char *most_popular_friend = calloc(MAX_COMMAND_LEN, 1);
	strcpy(most_popular_friend, ((friend_info *)user_friends_head->data)->friend_name);

	// compute the most popular friend among the user's friend
	while (user_friends_head) {
		friend_info *info = user_friends_head->data;
		unsigned int friend_id = get_user_id(info->friend_name);

		list_t *friend_friends_list = friends->graph->adjacency_list[friend_id];

		if (maximum_friends < friend_friends_list->size) {
			most_popular_friend[0] = 0;
			maximum_friends = friend_friends_list->size;
			strcpy(most_popular_friend, info->friend_name);
		} else {
			if (maximum_friends == friend_friends_list->size) {
				if (get_user_id(most_popular_friend) > friend_id) {
					most_popular_friend[0] = 0;
					maximum_friends = friend_friends_list->size;
					strcpy(most_popular_friend, info->friend_name);
				}
			}
		}

		user_friends_head = user_friends_head->next;
	}
	// compare the popularity between the user and the most popular among his friends

	unsigned int number_of_friends = user_friends->size;

	if (maximum_friends <= number_of_friends) {
		most_popular_friend[0] = 0;
		maximum_friends = number_of_friends;
		strcpy(most_popular_friend, (char *)key1);
		printf("%s is the most popular\n", most_popular_friend);
	} else {
		printf("%s is the most popular friend of %s\n", most_popular_friend, (char *)key1);
	}


	free(most_popular_friend);
}

void compute_suggestions(friends *friends, void *key1) {
	(void)key1;
	(void) friends;

	list_t *suggestions_list = NULL;

	create_list(&suggestions_list, sizeof(unsigned int), MAX_COMMAND_LEN, offsetof(friend_info, friend_id), offsetof(friend_info, friend_name), sizeof(friend_info), MAX_COMMAND_LEN, NULL);

	list_t *user_friends = friends->graph->adjacency_list[get_user_id(key1)];
	list_node_t *head = user_friends->head;

	while (head) {
		char *friend_name = ((friend_info *)(head->data))->friend_name;
		list_t *friend_friends = friends->graph->adjacency_list[get_user_id(friend_name)];
		list_node_t *friend_list_head = friend_friends->head;
		
		while (friend_list_head) {
			if (!is_my_friend(friends, key1, ((friend_info *)(friend_list_head->data))->friend_name) && strcmp(((friend_info *)(friend_list_head->data))->friend_name, (char *) key1)) {
				if (!get_item(2, &suggestions_list, ((friend_info *)(friend_list_head->data))->friend_name)) {
					list_node_t *new_node = calloc(1, sizeof(list_node_t));
					new_node->next = new_node->prev = NULL;
					new_node->data = calloc(1, suggestions_list->datasize);
					((friend_info *)new_node->data)->friend_name = calloc(MAX_COMMAND_LEN, 1);
					strcpy(((friend_info *)new_node->data)->friend_name, ((friend_info *)(friend_list_head->data))->friend_name);
					((friend_info *)new_node->data)->friend_id = get_user_id(((friend_info *)new_node->data)->friend_name);
	
					if (!suggestions_list->size) {
						suggestions_list->head = new_node;
						suggestions_list->tail = new_node;
					} else {
						list_node_t *suggestions_head = suggestions_list->head;
						list_node_t *tail = NULL;

						while (suggestions_head) {
							unsigned int list_node_friend_id = (((friend_info *)suggestions_head->data)->friend_id);
							unsigned int new_node_friend_id = get_user_id(((friend_info *)(friend_list_head->data))->friend_name);
	
							if (list_node_friend_id >= new_node_friend_id) {
								break;
							}
							tail = suggestions_head;
							suggestions_head = suggestions_head->next;
						}
	
						if (suggestions_head) {
							new_node->next = suggestions_head;
							new_node->prev = suggestions_head->prev;
							
							if (suggestions_head->prev) {
								suggestions_head->prev->next = new_node;
							} else {
								new_node->next = suggestions_list->head;
								suggestions_list->head->prev = new_node;
								suggestions_list->head = new_node;
	
							}
	
							suggestions_head->prev = new_node;
						} else {
							tail->next = new_node;
							new_node->prev = tail;
							new_node->next = NULL;
							tail = new_node;
						}
					}
	
					suggestions_list->size++;
				}
			}

			friend_list_head = friend_list_head->next;
		}
		head = head->next;
	}

	if (!suggestions_list->size) {
		printf("There are no suggestions for %s\n", (char *)key1);
	} else {
		printf("Suggestions for %s:\n", (char *)key1);

		list_node_t *head = suggestions_list->head;

		while (head) {
			friend_info *curr_friend_info = head->data;
			printf("%s\n", curr_friend_info->friend_name);

			head = head->next;
		}
	}

	free_list(2, &suggestions_list);
}

void remove_connection(friends **friends, void *key1, void *key2) {
	remove_entry(&(*friends)->graph, key1, key2);
	remove_entry(&(*friends)->graph, key2, key1);

	printf("Removed connection %s - %s\n", (char *)key1, (char *)key2);
}

void add_connection(friends **friends, void *key1, void *key2) {
	friend_info *new_info = calloc(1, sizeof(friend_info));
	new_info->friend_name = calloc(MAX_COMMAND_LEN, 1);
	strcpy(new_info->friend_name, (char*) key2);
	new_info->friend_id = get_user_id(new_info->friend_name);

	// adding entry for the first name
	add_entry(&(*friends)->graph, key1, new_info);

	friend_info *new_info_2 = calloc(1, sizeof(friend_info));
	new_info_2->friend_name = calloc(MAX_COMMAND_LEN, 1);
	strcpy(new_info_2->friend_name, (char*) key1);
	new_info->friend_id = get_user_id(new_info_2->friend_name);

	// adding entry for the second name
	add_entry(&(*friends)->graph, key2, new_info_2);

	printf("Added connection %s - %s\n", (char *)key1, (char *)key2);

	free(new_info->friend_name);
	new_info->friend_name = NULL;
	free(new_info);
	new_info = NULL;

	free(new_info_2->friend_name);
	new_info_2->friend_name = NULL;
	free(new_info_2);
	new_info_2 = NULL;
}

void fill_graph_keys(graph_t **graph, unsigned int size) {
	(*graph)->size = size - 1;
    for (uint16_t i = 0; i < size; i++) {
        strcpy((*graph)->adjacency_list[i]->key, get_user_name(i));
    }
}

friends *init_friends_network(unsigned int total_number_of_users) {
	friends *friends_network = calloc(1, sizeof(friends));

	friends_network->size = 0;
	// creating the graph base
	create_graph(&friends_network->graph, total_number_of_users, MAX_PEOPLE, MAX_COMMAND_LEN);

	// creating the graph adjacency list base
	create_graph_adjacency_list(&friends_network->graph, offsetof(friend_info, friend_id), sizeof(unsigned int), offsetof(friend_info, friend_name), MAX_COMMAND_LEN, sizeof(friend_info), MAX_COMMAND_LEN);

	// adding the names to the dictionary base
	fill_graph_keys(&friends_network->graph, total_number_of_users + 1);

	return friends_network;
}


u_int16_t retrieve_number_of_users(u_int16_t id) {
	while (!get_user_name(id)) {
		id--;
	}

	return id;
}

void handle_input_friends(char *input, friends **friends_network_main)
{
	char *commands = strdup(input);
	char *cmd = strtok(commands, "\n ");

	if (!cmd)
		return;

		
	friends *friends_network = *friends_network_main;
	u_int16_t network_size = 1000;

	if (!friends_network) {
		friends_network = init_friends_network(retrieve_number_of_users(network_size));
		(*friends_network_main) = friends_network;
	}

	if (!strcmp(cmd, "add")) {
		char *first_name = NULL, *second_name = NULL;
		cmd = strtok(NULL, "\n ");
		first_name = cmd;

		cmd = strtok(NULL, "\n ");
		second_name = cmd;

		if (first_name && second_name) {
			add_connection(&friends_network, first_name, second_name);
		}
	} else if (!strcmp(cmd, "remove")) {
		// TODO: Add function
		char *first_name = NULL, *second_name = NULL;
		cmd = strtok(NULL, "\n ");
		first_name = cmd;

		cmd = strtok(NULL, "\n ");
		second_name = cmd;

		if (first_name && second_name) {
			remove_connection(&friends_network, first_name, second_name);
		}
	} else if (!strcmp(cmd, "suggestions")) {
		// TODO: Add function
		char *name = NULL;
		cmd = strtok(NULL, "\n ");
		name = cmd;

		if (name) {
			compute_suggestions(friends_network, name);
		}
	} else if (!strcmp(cmd, "distance")) {
		// TODO: Add function
		char *first_name = NULL, *second_name = NULL;
		cmd = strtok(NULL, "\n ");
		first_name = cmd;

		cmd = strtok(NULL, "\n ");
		second_name = cmd;

		if (first_name && second_name) {
			compute_distance(friends_network, first_name, second_name);
		}
	} else if (!strcmp(cmd, "common")) {
		// TODO: Add function
		char *first_name = NULL, *second_name = NULL;
		cmd = strtok(NULL, "\n ");
		first_name = cmd;

		cmd = strtok(NULL, "\n ");
		second_name = cmd;

		if (first_name && second_name) {
			compute_common_friends(friends_network, first_name, second_name);
		}
	} else if (!strcmp(cmd, "friends")) {
		// TODO: Add function
		char *name = NULL;
		cmd = strtok(NULL, "\n ");
		name = cmd;

		if (name) {
			display_friends(friends_network, name);
		}
	} else if (!strcmp(cmd, "popular")) {
		// TODO: Add function
		char *name = NULL;
		cmd = strtok(NULL, "\n ");
		name = cmd;

		if (name) {
			compute_popular(friends_network, name);
		}
	}

	free(commands);
}
