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
	       "    -c CONFIG_FILE    specify a configuration file other than default\n"
	       "    -t ALARM          simulate an alarm\n"
	       "    -s                report a summary of configuration options\n"
	       "    -l                check for alarm conditions\n"
	       "    -h                display usage\n\n" "");

}

void check_alarms()
{
	int i;
	int cond_res;
	int action_res;

	for(i = 0; i < MAX_ELEMENTS; i++){

		if(conditions[i].name == NULL)
			break;

		cond_res = conditions[i].type->check_condition(&conditions[i]);

		if(cond_res == CONDITION_ON || cond_res == CONDITION_ERROR){
			struct action *action = conditions[i].action;
			action_res = action->type->trigger_action(action);
			if(action_res != ACTION_OK){
				printf("Action %s returned an ERROR!\n", action->name);
			}
		}

	}
}

int main(int argc, char **argv)
{
	int opt;
	char *config_file = "sysalarm.conf";

	while ((opt = getopt(argc, argv, "c:t:sl")) != -1) {
		switch (opt) {
		case 'c':
			config_file = optarg;
			break;
		case 't':
			break;
		case 's':
			break;
		case 'h':
			print_usage();
			exit(1);
			break;
		default:
			print_usage();
			exit(1);
		}
	}

	parse_config_file(config_file);
	check_alarms();

	return 0;
}
