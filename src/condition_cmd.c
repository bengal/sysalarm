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

struct cmd_cond_config {
	char *cmd_line;
	int cmd_timeout;
	int cmd_expected;
};

static void set_cmd_error(struct cmd_cond_config *config,
		struct result *result, char *message) {

	set_result(result, CONDITION_ERROR, "Error executing command '%s': %s",
			config->cmd_line, message);
}

static int cmd_cond_set_options(struct condition *condition, struct option_value *options)
{
	struct option_value *option;

	struct cmd_cond_config *config = calloc(1, sizeof(struct cmd_cond_config));
	CHECK_MALLOC(config);
	condition->priv_config = config;

	for (option = options; option != NULL; option = option->next) {

		if (!strcmp(option->name, "cmd_line")) {
			config->cmd_line = strdup(option->value);

		} else if (!strcmp(option->name, "cmd_timeout")) {
			config->cmd_timeout = atoi(option->value);

		}  else if (!strcmp(option->name, "cmd_expected")) {
			config->cmd_expected = atoi(option->value);

		} else {
			die("Unknown option '%s' for condition '%s'", option->name, condition->name);
		}
	}

	if(config->cmd_line == NULL)
		die("You must supply 'cmd_line' parameter for condition of type '%s'",
				condition->type->name);

	return 0;
}

static void check_return_value(struct cmd_cond_config *config, int status,
		struct result *result)
{
	if(WIFEXITED(status) && WEXITSTATUS(status) == config->cmd_expected) {
		set_result(result, CONDITION_OFF, NULL);
		return;
	} else {
		set_result(result, CONDITION_ON, "The command '%s' exited "
				" with code %d (expected: %d)", config->cmd_line,
				WEXITSTATUS(status), config->cmd_expected);
		return;
	}
}

static void wait_for_child(pid_t pid, struct cmd_cond_config *config, struct result *result)
{
	pid_t ret_pid;
	int status;
	time_t start_time = time(NULL);
	time_t current_time;

	while (1) {
		/* check if the child has terminated */
		ret_pid = waitpid(pid, &status, config->cmd_timeout == 0 ? 0 : WNOHANG);

		if (ret_pid > 0) {
			check_return_value(config, status, result);
			return;
		} else if (ret_pid == -1) {
			set_cmd_error(config, result, "Internal error: waitpid()");
			return;
		}

		/* child process hasn't terminated yet */
		current_time = time(NULL);

		if (current_time - start_time > config->cmd_timeout) {
			sa_log(SA_LOG_DEBUG, "Command timeout, killing child process\n");
			kill(pid, SIGKILL);
			set_cmd_error(config, result, "Command timeout");
			return;
		}
		sleep(1);
	}
}

static void cmd_cond_check_condition(struct condition *condition, struct result *result)
{
	struct cmd_cond_config *config = condition->priv_config;
	pid_t pid;

	if((pid = fork()) == -1){

		set_cmd_error(config, result, "fork()");
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
		wait_for_child(pid, config, result);
	}

}

struct condition_type condition_type_cmd = {
	.name = "CMD",
	.description = "Check the exit value of a command",
	.set_options = cmd_cond_set_options,
	.check_condition = cmd_cond_check_condition,
};
