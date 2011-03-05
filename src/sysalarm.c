#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"

#include "base.h"
#include "parse.h"
#include "util.h"

void print_usage()
{


	printf(
		"sysalarm is a tool for monitoring the system for particular\n"
		"events ('conditions') and do something ('action') when they\n"
		"happen.\n"
		"The program includes a predefined set of conditions and actions\n"
		"that you can use and configure to suite your needs\n\n"

		"Usage: sysalarm [OPTION]...\n\n"

		"Options:\n"
		"   -c CONFIG_FILE    specify a configuration file other than default\n"
		"   -t COND_NAME      simulate an alarm condition (trigger associated action)\n"
		"   -s                print a summary of configuration options\n"
		"   -a                check for alarm conditions (don't trigger actions)\n"
		"   -l                list available condition and action types\n"
		"   -v, --version     output version information and exit\n"
		"   -h, --help        display this help and exit\n\n"

		"Examples:\n"
		"  sysalarm           checks alarms using default configuration file\n"
		"  sysalarm -t AL1    simulate alarm AL1 defined in configuration\n\n"

		"Report bugs to " PACKAGE_BUGREPORT "\n\n"

			);


}

void print_version()
{
	printf("sysalarm v" PACKAGE_VERSION "\n\n"
		    "Copyright (C) 2011 Beniamino Galvani.\n\n"
		    "This is free software; see the source for copying conditions.  There is NO\n"
		    "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n");

}

void check_alarms(int trigger_action)
{
	int i;
	struct result cond_res;
	struct result action_res;

	for(i = 0; i < MAX_ELEMENTS; i++){

		if(conditions[i].name == NULL)
			break;

		memset(&cond_res, 0, sizeof(struct result));
		conditions[i].type->check_condition(&conditions[i], &cond_res);

		if(cond_res.code == CONDITION_ON || cond_res.code == CONDITION_ERROR){

			if(trigger_action){
				struct action *action = conditions[i].action;
				action->type->trigger_action(action, &cond_res, &action_res);
				if(action_res.code != ACTION_OK){
					printf("Action %s returned an ERROR!\n", action->name);
				}
			} else {
				printf("ALARM CONDITION: %s (msg: %s)(type:%s)\n", conditions[i].name,
						cond_res.desc,
						conditions[i].type->name);
			}
		}

	}
}

void simulate_alarm(char *condition_name)
{
	struct result action_result;
	struct result fake_cond_result;
	struct condition *cond = search_condition(condition_name);

	if(!cond)
		die("Condition '%s' not found", condition_name);

	memset(&fake_cond_result, 0, sizeof(struct result));
	memset(&action_result, 0, sizeof(struct result));

	fake_cond_result.code = CONDITION_ON;
	snprintf(fake_cond_result.desc, RESULT_DESC_LEN,
			"alarm simulation for condition: %s", condition_name);

	cond->action->type->trigger_action(cond->action, &fake_cond_result, &action_result);
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

extern struct condition_type *condition_types[];
extern struct condition_type *action_types[];
void print_types()
{
	int i;

	printf("==== CONDITION TYPES =====\n");

	for(i = 0; ; i++){
		if(condition_types[i]->name == NULL)
			break;
		printf("%-20s %-60s\n", condition_types[i]->name, condition_types[i]->description);
	}

	printf("==== ACTION TYPES =====\n");

	for(i = 0; ; i++){
		if(action_types[i]->name == NULL)
			break;
		printf("%-20s %-60s\n", action_types[i]->name, action_types[i]->description);
	}
}

struct option long_options[] = {
		{"version", 0, 0, 'v'},
		{"help", 0, 0, 'h'},
		{0, 0, 0, 0}
};

int main(int argc, char **argv)
{
	int opt, option_index = 0;
	char *config_file = "sysalarm.conf";
	char *simul_cond = NULL;
	int summary = 0;
	int list = 0;
	int types = 0;

	while ((opt = getopt_long(argc, argv, "c:t:slahv", long_options, &option_index)) != -1) {
		switch (opt) {
		case 'l':
			types = 1;
			break;
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
		case 'a':
			list = 1;
			break;
		case 'v':
			print_version();
			exit(0);
		default:
			print_usage();
			exit(0);
		}
	}

	parse_config_file(config_file);

	if(types){
		print_types();
		exit(0);
	}

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
