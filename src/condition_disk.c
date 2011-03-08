/*
 * alarm_disk.c
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/vfs.h>

#include "base.h"
#include "util.h"

struct disk_cond_config {
	char *disk_file;
	int disk_threshold;
};

static int disk_cond_set_options(struct condition *condition, struct option_value *options)
{
	struct option_value *option;

	struct disk_cond_config *config = calloc(1, sizeof(struct disk_cond_config));
	CHECK_MALLOC(config);
	condition->specific_config = config;

	for (option = options; option != NULL; option = option->next) {

		if(!option->specific)
			continue;

		if (!strcmp(option->name, "disk_file")) {
			config->disk_file = strdup(option->value);
		} else if (!strcmp(option->name, "disk_threshold")) {
			config->disk_threshold = atoi(option->value);
		} else {
			die("Unknown option '%s' for condition '%s'", option->name, condition->name);
		}
	}

	if(config->disk_file == NULL || config->disk_threshold == 0)
		die("Disk condition: you must supply 'disk_file' and 'disk_threshold' parameters");

	return 0;
}

static void disk_cond_check_condition(struct condition *condition, struct result *result)
{
	struct disk_cond_config *config = condition->specific_config;
	struct statfs stat;
	long disk_usage;
	unsigned int usage;

	if (statfs(config->disk_file, &stat) == -1){
		set_result(result, CONDITION_ERROR, "Error accessing file %s", config->disk_file);
		return;
	}

	disk_usage = stat.f_bfree * 100 / stat.f_blocks;

	debug("Disk usage for %s : %ld / %ld = %ld\n", config->disk_file, stat.f_bfree,
	      stat.f_blocks, disk_usage);

	usage = disk_usage;

	if (usage >= config->disk_threshold){
		set_result(result, CONDITION_ON, "Disk usage for %s : %ld %%",
				config->disk_file, disk_usage);
		return;
	}

	set_result(result, CONDITION_OFF, NULL);
}


struct condition_type condition_type_disk = {
	.name = "DISK",
	.description = "Checks disk usage",
	.set_options = disk_cond_set_options,
	.check_condition = disk_cond_check_condition,
};
