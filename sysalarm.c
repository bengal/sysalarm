#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>

#include "config.h"
#include "parse.h"
#include "util.h"

void print_usage()
{
	printf("Usage: sysalarm [OPTIONS]\n\n"
	       "    -c CONFIG_FILE  specify a configuration file other than default\n"
	       "    -t COND_NAME    simulate an alarm condition (trigger associated action)\n"
	       "    -s              print a summary of configuration options\n"
	       "    -l              check for alarm conditions (don't trigger actions)\n"
	       "    -h              display usage\n\n" "");

}

void check_alarms(int tr_action)
{
	int i;
	int cond_res;
	int action_res;

	for(i = 0; i < MAX_ELEMENTS; i++){

		if(conditions[i].name == NULL)
			break;

		cond_res = conditions[i].type->check_condition(&conditions[i]);

		if(cond_res == CONDITION_ON || cond_res == CONDITION_ERROR){
			if(tr_action){
				struct action *action = conditions[i].action;
				action_res = action->type->trigger_action(action);
				if(action_res != ACTION_OK){
					printf("Action %s returned an ERROR!\n", action->name);
				}
			} else {
				printf("ALARM CONDITION: %s (type:%s)\n", conditions[i].name,
						conditions[i].type->name);
			}
		}

	}
}

void simulate_alarm(char *condition_name)
{
	struct condition *cond = search_condition(condition_name);
	if(!cond)
		die("Condition '%s' not found", condition_name);
	cond->action->type->trigger_action(cond->action);
}

void print_config_summary()
{
	int i;

	printf("===== ACTIONS ========\n");

	for(i = 0; i < MAX_ELEMENTS; i++){

		if(actions[i].name == NULL)
			break;

		printf(" * %-20s type: %-15s\n", actions[i].name, actions[i].type->name);
	}

	printf("===== CONDITIONS =====\n");

	for(i = 0; i < MAX_ELEMENTS; i++){

		if(conditions[i].name == NULL)
			break;

		printf(" * %-20s type: %-15s action: %-20s\n",
				conditions[i].name, conditions[i].type->name, conditions[i].action->name);
	}
}

int main(int argc, char **argv)
{
	int opt;
	char *config_file = "sysalarm.conf";
	char *simul_cond = NULL;
	int summary = 0;
	int list = 0;

	while ((opt = getopt(argc, argv, "c:t:sl")) != -1) {
		switch (opt) {
		case 'c':
			config_file = optarg;
			break;
		case 't':
			simul_cond = optarg;
			break;
		case 's':
			summary = 1;
			break;
		case 'h':
			print_usage();
			exit(0);
			break;
		case 'l':
			list = 1;
			break;
		default:
			print_usage();
			exit(0);
		}
	}

	parse_config_file(config_file);

	if(summary){
		print_config_summary();
		exit(0);
	}

	if(simul_cond){
		simulate_alarm(simul_cond);
		exit(0);
	}

	if(list){
		check_alarms(0);
		exit(0);
	}

	check_alarms(1);

	return 0;
}
