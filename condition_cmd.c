/*
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>


#include "base.h"
#include "util.h"

struct cmd_condition_config {
	char *cmd_line;
	int timeout;
	int expected;
};

static int cmd_cond_set_options(struct condition *condition, struct option_value *options)
{
	struct option_value *option;

	struct cmd_condition_config *config = calloc(1, sizeof(struct cmd_condition_config));
	CHECK_MALLOC(config);
	condition->specific_config = config;

	for (option = options; option != NULL; option = option->next) {

		if(!option->specific)
			continue;

		if (!strcmp(option->name, "cmd_line")) {
			config->cmd_line = strdup(option->value);

		} else if (!strcmp(option->name, "timeout")) {
			config->timeout = atoi(option->value);

		}  else if (!strcmp(option->name, "expected")) {
			config->expected = atoi(option->value);

		} else {
			die("Unknown option '%s' for condition '%s'", option->name, condition->name);
		}
	}

	if(config->cmd_line == NULL)
		die("You must supply 'cmd_line' parameter for condition of type '%s'",
				condition->type->name);

	return 0;
}

static void check_return_value(struct cmd_condition_config *config, int status,
		struct result *result)
{
	if(status == config->expected){
		result->code = CONDITION_OFF;
		return;
	} else {
		result->code = CONDITION_ON;
		snprintf(result->desc, RESULT_DESC_LEN, "The command '%s' exited "
				" with code %d (expected: %d)",
				config->cmd_line, status, config->expected);
		return;
	}
}

static void set_error(struct cmd_condition_config *config, struct result *result,
		char *message)
{
	result->code = CONDITION_ERROR;
	snprintf(result->desc, RESULT_DESC_LEN, "Error executing command '%s': %s",
			config->cmd_line, message);
}

static void cmd_cond_check_condition(struct condition *condition, struct result *result)
{
	struct cmd_condition_config *config = condition->specific_config;
	pid_t pid;
	int status;

	if((pid = fork()) == -1){

		set_error(config, result, "fork()");
		return;

	} else if(pid == 0){
		/* child: launch command */
		char *line = strdup(config->cmd_line);
		char **args = calloc(1, sizeof(char *) * MAX_ELEMENTS);
		char *arg = strtok(line, " ");
		int count = 0;
		int result;

		while(arg && count < MAX_ELEMENTS){
			args[count++]  = arg;
			arg = strtok(NULL, " ");
		}

		result = execvp(args[0], args);
		exit(-1);

	} else {
		/* parent */

		time_t start_time = time(NULL);
		time_t current_time;
		pid_t ret_pid;

		while(1){
			/* check if the child has terminated */
			ret_pid = waitpid(pid, &status, WNOHANG);

			if(ret_pid > 0){
				check_return_value(config, status, result);
				return;
			} else if(ret_pid == -1){
				set_error(config, result, "Internal error: waitpid()");
				return;
			}

			/* child process hasn't terminated yet */
			current_time = time(NULL);

			if(current_time - start_time > config->timeout){

				debug("Command timeout, killing child process\n");

				/* kill child, sorry I must do it */
				kill(pid, SIGKILL);

				set_error(config, result, "Command timeout");
				return;
			}
			sleep(1);
		}

	}

}


struct condition_type condition_type_cmd = {
	.name = "CMD",
	.set_options = cmd_cond_set_options,
	.check_condition = cmd_cond_check_condition,
};
