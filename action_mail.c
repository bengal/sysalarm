/*
 * action_mail.c
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "util.h"

#define METHOD_LOCAL 1
#define METHOD_SMTP 2

struct mail_action_config {
	char *mail_from;
	char *mail_to;
	char *mail_subject;
	int mail_method;
};

void check_configuration(struct mail_action_config *config)
{

}


int mail_action_set_options(struct action *action, struct option_value *options)
{
	struct option_value *option;
	struct mail_action_config *config = malloc(sizeof(struct mail_action_config));
	action->specific_config = config;

	for (option = options; option != NULL; option = option->next) {

		if(!option->specific)
			continue;

		if (!strcmp(option->name, "mail_from")) {
			config->mail_from = strdup(option->value);

		} else if (!strcmp(option->name, "mail_to")) {
			config->mail_to = strdup(option->value);

		} else if (!strcmp(option->name, "mail_subject")) {
			config->mail_subject = strdup(option->value);

		} else if (!strcmp(option->name, "mail_method")) {

			if(!strcmp(option->value, "local")){
				config->mail_method = METHOD_LOCAL;

			} else if(!strcmp(option->value, "smtp")){
				config->mail_method = METHOD_SMTP;

			} else {
				die("Unknown mail method %s", option->value);
			}
		} else {
			die("Unknown option '%s' for action '%s'", option->name, action->name);
		}
	}

	check_configuration(config);

	return 0;
}


int mail_action_trigger_action(struct action *action)
{
	debug("Sending an email... DONE\n");

	return 0;
}

struct action_type action_type_mail = {
	.name = "MAIL",
	.set_options = mail_action_set_options,
	.trigger_action = mail_action_trigger_action,
};
