
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "base.h"
#include "util.h"

struct cmd_action_config {
	char *cmd_line;
	int cmd_backgr;
};

static void set_cmd_error(struct cmd_action_config *config,
		struct result *result, char *message) {

	set_result(result, ACTION_ERROR, "Error executing command '%s': %s",
			config->cmd_line, message);

}

static void cmd_action_trigger_action(struct action *action, struct result *cond_res,
		struct result *result)
{
	struct cmd_action_config *config = action->priv_config;
	pid_t pid;

	if (config->cmd_backgr) {
		if ((pid = fork()) == -1) {
			set_cmd_error(config, result, "fork()");
			return;
		} else if (pid == 0) {
			/* child: launch command */
			char *line = strdup(config->cmd_line);
			char **args = calloc(1, sizeof(char *) * MAX_ELEMENTS);
			char *arg = strtok(line, " ");
			int count = 0;
			int result;

			while (arg && count < MAX_ELEMENTS) {
				args[count++] = arg;
				arg = strtok(NULL, " ");
			}

			result = execvp(args[0], args);
			exit(-1);
		}
	} else {
		if (system(config->cmd_line) == -1) {
			set_cmd_error(config, result, "error in system()");
			return;
		}
	}

	set_result(result, ACTION_OK, NULL);
}

static int cmd_action_set_options(struct action *action, struct option_value *options)
{
	struct option_value *option;
	struct cmd_action_config *config = calloc(1, sizeof(struct cmd_action_config));
	action->priv_config = config;

	CHECK_MALLOC(config);

	for(option = options; option != NULL; option = option->next){

		if(!strcmp(option->name, "cmd_line")){
			config->cmd_line = strdup(option->value);
		} else if(!strcmp(option->name, "cmd_backgr")){
			config->cmd_backgr = atoi(option->value);
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
	.description = "Executes a command",
	.set_options = cmd_action_set_options,
	.trigger_action = cmd_action_trigger_action,
};
