/*
 * alarm_disk.c
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/vfs.h>

#include "config.h"
#include "util.h"

struct disk_condition_config {
	char *device;
	int threshold;
};

static int disk_cond_set_options(struct condition *condition,
				 struct option_value *options)
{
	struct option_value *option;

	struct disk_condition_config *config =
	    malloc(sizeof(struct disk_condition_config));
	condition->specific_config = config;

	for (option = options; option != NULL; option = option->next) {
		if (!strcmp(option->name, "name")) {
			/* TODO remove these common options in parse.c */
		} else if (!strcmp(option->name, "action")) {
			/* TODO remove these common options in parse.c */
		} else if (!strcmp(option->name, "type")) {
			/* TODO remove these common options in parse.c */
		} else if (!strcmp(option->name, "device")) {
			config->device = strdup(option->value);
		} else if (!strcmp(option->name, "threshold")) {
			config->threshold = atoi(option->value);
		} else {
			die("Unknown option '%s' for condition '%s'", option->name,
			    condition->name);
		}
	}

	return 0;
}

static int disk_cond_check_alarm(struct condition *condition)
{
	struct disk_condition_config *config = condition->specific_config;
	struct statfs stat;

	if (statfs(config->device, &stat) == -1)
		return CONDITION_ERROR;

	long disk_usage = stat.f_bfree * 100 / stat.f_blocks;
	debug("Disk usage for %s : %ld / %ld = %ld\n", config->device, stat.f_bfree,
	      stat.f_blocks, disk_usage);

	unsigned int usage = disk_usage;

	if (usage >= config->threshold)
		return CONDITION_ON;

	return CONDITION_OFF;
}


struct condition_type condition_type_disk = {
	.name = "disk",
	.set_options = disk_cond_set_options,
	.check_condition = disk_cond_check_alarm,
};
