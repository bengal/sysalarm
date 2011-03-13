/*
 * parse.c
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "parse.h"
#include "base.h"
#include "util.h"


int is_blank_line(char *line)
{
	char *ptr = line;
	while (*ptr != 0 && *ptr != '\n') {

		if (*ptr == '#')
			return 1;

		if (!isspace(*ptr))
			return 0;

		ptr++;
	}
	return 1;
}

int is_end_section(char *str)
{
	char *ptr = str;

	while (isspace(*ptr))
		ptr++;

	if (*ptr != '}')
		return 0;

	ptr++;
	while (isspace(*ptr))
		ptr++;

	if (*ptr == '\n' || *ptr == 0)
		return 1;

	return 0;
}

struct option_value *search_option(struct option_value *head, char *name)
{
	struct option_value *opt;
	for (opt = head; opt != NULL; opt = opt->next) {
		if (!strcmp(opt->name, name)) {
			return opt;
		}
	}
	return NULL;
}

char *parse_section_name(char *str)
{
	char *name = strtok(str, " \t");
	if (name != NULL) {
		char *delim = strtok(NULL, " \n");
		if (delim != NULL && strcmp(delim, "{") == 0) {
			return strdup(name);
		}
	}

	return NULL;
}

char *trim(char *str)
{
	char *ptr = str + strlen(str) - 1;
	while (*ptr == ' ' || *ptr == '\t' || *ptr == '\n') {
		*ptr = 0;
		ptr--;
	}

	ptr = str;
	while (*ptr != 0 && (*ptr == ' ' || *ptr == '\t')) {
		ptr++;
	}

	return ptr;
}

void add_option(struct option_value **options, char *name, char *value)
{
	struct option_value *last;
	struct option_value *new_opt;

	new_opt = malloc(sizeof(struct option_value));
	CHECK_MALLOC(new_opt);
	new_opt->name = strdup(name);
	new_opt->value = strdup(value);
	new_opt->next = NULL;
	new_opt->specific = 1;

	if (*options == NULL) {
		*options = new_opt;
	} else {
		for (last = *options; last->next != NULL; last = last->next);
		last->next = new_opt;
	}
}

void parse_cond_actions(struct condition *cond, char *str)
{
	char *line = strdup(str);
	char *ptr = line;
	int ntokens = 1;
	int count = 0;

	while(*ptr){
		if(*ptr == ',')
			ntokens++;
		ptr++;
	}

	cond->actions = calloc(ntokens + 1, sizeof(struct cond_reg_action *));
	ptr = strtok(str, ",");
	if(!ptr)
		die("You must specify at least an action for every condition");

	while(ptr){
		struct action *action;
		struct cond_reg_action *cra;
		int stop = 0;

		ptr = trim(ptr);
		if(*ptr == '#'){
			stop = 1;
			ptr++;
		}
		action  = search_action(ptr);

		if(!action)
			die("Action '%s' is undefined\n", ptr);

		cra = malloc(sizeof(struct cond_reg_action));
		CHECK_MALLOC(cra);

		cra->action = action;
		cra->stop = stop;

		cond->actions[count++] = cra;
		ptr = strtok(NULL, ",");
	}

}

void create_new_condition(struct option_value *options)
{
	struct condition *condition = new_condition();
	struct option_value *option;
	struct condition_type *type;

	if (!condition)
		die("Too many conditions");

	/* Initialize condition name */
	option = search_option(options, "name");

	if(!option)
		die("You must specify a name for every condition");

	option->specific = 0;
	condition->name = option->value;

	/* Initialize condition action */
	option = search_option(options, "action");

	if(!option)
		die("You must specify at least an action for every condition");

	option->specific = 0;
	parse_cond_actions(condition, option->value);

	/* Initialize condition type */
	option = search_option(options, "type");

	if (!option)
		die("You must specify a type for every condition");

	option->specific = 0;
	type = search_condition_type(option->value);

	if (!type)
		die("The condition type '%s' does not exist", option->value);

	condition->type = type;

	option = search_option(options, "hold_time");
	if (option) {
		option->specific = 0;
		condition->hold_time = atoi(option->value);
	}

	option = search_option(options, "inactive_time");
	if (option) {
		option->specific = 0;
		condition->hold_time = atoi(option->value);
	}

	type->set_options(condition, options);

}

void create_new_action(struct option_value *options)
{
	struct action *action = new_action();
	struct option_value *option;
	struct action_type *type;

	if (!action)
		die("Too many actions");

	/* Initialize action name */
	option = search_option(options, "name");

	if(!option)
		die("You must specify a name for every condition");

	option->specific = 0;
	action->name = option->value;

	/* Initialize action type */
	option = search_option(options, "type");

	if (!option)
		die("You must specify a type for every action");

	option->specific = 0;
	type = search_action_type(option->value);

	if (!type)
		die("The action type '%s' does not exist", option->value);

	action->type = type;
	type->set_options(action, options);

}

void parse_config_file(char *file_name)
{
	FILE *file;
	char *line;
	ssize_t read;
	size_t len;
	int line_num = 0;
	char *current_section = NULL;
	struct option_value *options = NULL;
	char *sep;
	char *name, *value;

	debug("Parsing file %s\n", file_name);

	if ((file = fopen(file_name, "r")) == NULL) {
		die("Error opening configuration file %s\n", file_name);
	}

	line = malloc(BUF_LEN);
	CHECK_MALLOC(line);

	while ((read = getline(&line, &len, file)) != -1) {

		line_num++;

		if (is_blank_line(line))
			continue;

		if (current_section == NULL) {
			current_section = parse_section_name(line);
			if (current_section == NULL)
				die("Expected start of section at line %d", line_num);
			continue;
		}

		if (is_end_section(line)) {

			if (!strcmp(current_section, "condition")) {
				create_new_condition(options);
			} else if (!strcmp(current_section, "action")) {
				create_new_action(options);
			} else {
				die("Unknown section name %s at line %d", current_section,
				    line_num);
			}

			options = NULL;
			current_section = NULL;
			continue;
		}

		sep = strchr(line, '=');
		if (sep == NULL) {
			die("Syntax error at line %d\n", line_num);
		}

		name = trim(strtok(line, "="));
		value = trim(strtok(NULL, ""));

		add_option(&options, name, value);

	}
}
