/*
 * parse.c
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>


#include "parse.h"
#include "config.h"
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
	for(opt = head; opt != NULL; opt = opt->next){
		if(!strcmp(opt->name, name)){
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
	new_opt->name = strdup(name);
	new_opt->value = strdup(value);
	new_opt->next = NULL;

	if(*options == NULL){
		*options = new_opt;
	} else {
		for(last = *options; last->next != NULL; last = last->next);
		last->next = new_opt;
	}
}

void create_new_condition(struct option_value *options)
{
	struct condition *condition = new_condition();
	struct option_value *type_option;
	struct condition_type *type;

	if(!condition)
		die("Too many conditions");

	type_option = search_option(options, "type");

	if(!type_option)
		die("You must specify a type for every condition");

	type = search_condition_type(type_option->value);

	if(!type)
		die("The condition type '%s' does not exist", type_option->value);

	condition->type = type;
	type->set_options(condition, options);
}

void create_new_action(struct option_value *options)
{
	struct action *action = new_action();
	struct option_value *type_option;
	struct action_type *type;

	if(!action)
		die("Too many actions");

	type_option = search_option(options, "type");

	if(!type_option)
		die("You must specify a type for every action");

	type = search_action_type(type_option->value);

	if(!type)
		die("The action type '%s' does not exist", type_option->value);

	action->type = type;
	type->set_options(action, options);
}

void parse_config_file(char *file_name)
{
	FILE *file;
	char *line;
	ssize_t read, len;
	int line_num = 0;
	char *current_section = NULL;
	struct option_value *options = NULL;

	debug("Parsing file %s\n", file_name);

	if ((file = fopen(file_name, "r")) == NULL) {
		die("Error opening configuration file %s\n", file_name);
	}

	line = malloc(BUF_LEN);

	while ((read = getline(&line, &len, file)) != -1) {

		line_num++;

		if (is_blank_line(line))
			continue;

		if (current_section == NULL) {
			current_section = parse_section_name(line);
			if(current_section == NULL)
				die("Expected start of section at line %d", line_num);
			continue;
		}

		if (is_end_section(line)) {

			if(!strcmp(current_section, "condition")){
				create_new_condition(options);
			} else if(!strcmp(current_section, "action")){
				create_new_action(options);
			} else {
				die("Unknown section name %s at line %d", current_section, line_num);
			}

			current_section = NULL;
			continue;
		}

		char *sep = strchr(line, '=');
		if (sep == NULL) {
			die("Syntax error at line %d\n", line_num);
		}

		char *name = trim(strtok(line, "="));
		char *value = trim(strtok(NULL, ""));

		add_option(&options, name, value);

	}
}



