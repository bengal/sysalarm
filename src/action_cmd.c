
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "base.h"
#include "util.h"

struct cmd_action_config {
	char *cmd_line;

	/* TODO add option for non-blocking execution */

};

static void cmd_action_trigger_action(struct action *action, struct result *cond_res,
		struct result *result)
{
	struct cmd_action_config *config = action->specific_config;

	if(system(config->cmd_line) == -1){
		result->code = ACTION_ERROR;
		snprintf(result->desc, RESULT_DESC_LEN, "Error launching command: %s",
				config->cmd_line);
		return;
	}

	result->code = ACTION_OK;
}


static int cmd_action_set_options(struct action *action, struct option_value *options)
{
	struct option_value *option;
	struct cmd_action_config *config = calloc(1, sizeof(struct cmd_action_config));
	action->specific_config = config;

	CHECK_MALLOC(config);

	for(option = options; option != NULL; option = option->next){

		if(!option->specific)
			continue;

		if(!strcmp(option->name, "cmd_line")){
			config->cmd_line = strdup(option->value);
		} else {
			die("Unknown option '%s' for action '%s'", option->name, action->name);
		}
	}

	if(config->cmd_line == NULL)
		die("Parameter 'cmd_line' is required for action %s", action->name);

	return 0;

}

struct action_type action_type_cmd = {
	.name = "CMD",
	.set_options = cmd_action_set_options,
	.trigger_action = cmd_action_trigger_action,
};
