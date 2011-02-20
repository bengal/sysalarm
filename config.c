/*
 * config2.c
 */


#include <stdio.h>
#include <string.h>

#include "config.h"

struct condition conditions[MAX_ELEMENTS];
struct action actions[MAX_ELEMENTS];

struct condition_type dummy_condition_type = {.name = NULL};

struct condition_type *condition_types[] = {
		&dummy_condition_type
};

struct action_type dummy_action_type = {.name = NULL};

struct action_type *action_types[] = {
		&dummy_action_type
};

/* Is this really needed ? */
void initialize_structs()
{
	memset(conditions, 0, sizeof(struct condition) * MAX_ELEMENTS);
	memset(actions, 0, sizeof(struct action) * MAX_ELEMENTS);
}

struct condition *new_condition()
{
	int i;
	for(i = 0; i < MAX_ELEMENTS; i++){
		if(conditions[i].name == NULL)
			return &conditions[i];
	}
	return NULL;
}

struct action *new_action()
{
	int i;
	for(i = 0; i < MAX_ELEMENTS; i++){
		if(actions[i].name == NULL)
			return &actions[i];
	}
	return NULL;
}

struct condition *search_condition(char *name)
{
	int i;
	for(i = 0; i < MAX_ELEMENTS; i++){

		if(conditions[i].name == NULL)
			break;

		if(!strcmp(name, conditions[i].name))
			return &conditions[i];
	}
	return NULL;
}

struct action *search_action(char *name)
{
	int i;
	for(i = 0; i < MAX_ELEMENTS; i++){

		if(actions[i].name == NULL)
			break;

		if(!strcmp(name, actions[i].name))
			return &actions[i];
	}
	return NULL;
}

struct condition_type *search_condition_type(char *name)
{
	int i;
	for(i = 0; ; i++){

		if(condition_types[i]->name == NULL)
			break;

		if(!strcmp(name, condition_types[i]->name))
			return condition_types[i];
	}
	return NULL;
}

struct action_type *search_action_type(char *name)
{
	int i;
	for(i = 0; ; i++){

		if(action_types[i]->name == NULL)
			break;

		if(!strcmp(name, action_types[i]->name))
			return action_types[i];
	}
	return NULL;
}




