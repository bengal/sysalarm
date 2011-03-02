/*
 * base.c
 */


#include <stdio.h>
#include <string.h>

#include "base.h"

struct condition conditions[MAX_ELEMENTS];
struct action actions[MAX_ELEMENTS];

extern struct condition_type condition_type_disk;
extern struct condition_type condition_type_tcp;
extern struct condition_type condition_type_cmd;
struct condition_type condition_type_dummy = { .name = NULL };

struct condition_type *condition_types[] = {
	&condition_type_disk,
	&condition_type_tcp,
	&condition_type_cmd,
	&condition_type_dummy
};

extern struct action_type action_type_mail;
extern struct action_type action_type_cmd;
struct action_type action_type_dummy = { .name = NULL };

struct action_type *action_types[] = {
	&action_type_mail,
	&action_type_cmd,
	&action_type_dummy
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
	for (i = 0; i < MAX_ELEMENTS; i++) {
		if (conditions[i].name == NULL)
			return &conditions[i];
	}
	return NULL;
}

struct action *new_action()
{
	int i;
	for (i = 0; i < MAX_ELEMENTS; i++) {
		if (actions[i].name == NULL)
			return &actions[i];
	}
	return NULL;
}

struct condition *search_condition(char *name)
{
	int i;
	for (i = 0; i < MAX_ELEMENTS; i++) {

		if (conditions[i].name == NULL)
			break;

		if (!strcmp(name, conditions[i].name))
			return &conditions[i];
	}
	return NULL;
}

struct action *search_action(char *name)
{
	int i;
	for (i = 0; i < MAX_ELEMENTS; i++) {

		if (actions[i].name == NULL)
			break;

		if (!strcmp(name, actions[i].name))
			return &actions[i];
	}
	return NULL;
}

struct condition_type *search_condition_type(char *name)
{
	int i;
	for (i = 0;; i++) {

		if (condition_types[i]->name == NULL)
			break;

		if (!strcmp(name, condition_types[i]->name))
			return condition_types[i];
	}
	return NULL;
}

struct action_type *search_action_type(char *name)
{
	int i;
	for (i = 0;; i++) {

		if (action_types[i]->name == NULL)
			break;

		if (!strcmp(name, action_types[i]->name))
			return action_types[i];
	}
	return NULL;
}

char *result_get_description(struct result *result)
{
	return result->ext_desc == NULL ? result->desc : result->ext_desc;
}
