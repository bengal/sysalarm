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
	       "    -h                display usage\n" "");

}

void check_alarms()
{
	int i;
	int result;

	for(i = 0; i < MAX_ELEMENTS; i++){

		if(conditions[i].name == NULL)
			break;

		result = conditions[i].type->check_condition(&conditions[i]);

		if(result == CONDITION_ON || result == CONDITION_ERROR){
			struct action *action = conditions[i].action;
			action->type->trigger_action(action);
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
