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

struct disk_alarm_options {
	char *device;
	int threshold;
};


static int disk_check_alarm(struct alarm_condition *config)
{
	struct disk_alarm_options *opt = SPECIFIC_CONFIG(opt, config);
	struct statfs stat;

	if (statfs(opt->device, &stat) == -1)
		return ALARM_ERROR;

	long disk_usage = stat.f_bfree * 100 / stat.f_blocks;
	debug("Disk usage for %s : %ld / %ld = %ld\n", opt->device, stat.f_bfree,
	      stat.f_blocks, disk_usage);

	unsigned int usage = disk_usage;
	if (usage >= opt->threshold)
		return ALARM_ON;

	return ALARM_OFF;
}



static int disk_parse_config_option(struct alarm_condition *config, char *key,
				    char *value)
{
	struct disk_alarm_options *opt = SPECIFIC_CONFIG(opt, config);

	if (!strcmp(key, "device")) {
		opt->device = strdup(value);
		return 0;
	} else if (!strcmp(key, "threshold")) {
		opt->threshold = atoi(value);
		return 0;
	}

	return -1;
}

static void disk_check_config(struct alarm_condition *config)
{
	struct disk_alarm_options *opt = SPECIFIC_CONFIG(opt, config);

	if (opt->device == NULL) {
		die("Alarm %s (type %s) requires parameter 'device'\n", config->name,
		    config->type->code);
	}

	if (opt->threshold < 1 || opt->threshold > 99) {
		die("Alarm %s (type %s) requires parameter 'threshold'\n", config->name,
		    config->type->code);
	}
}

static void disk_init_alarm_config(struct alarm_condition *config)
{
	config->specific_config = malloc(sizeof(struct disk_alarm_options));
	memset(config->specific_config, 0, sizeof(struct disk_alarm_options));
}

struct alarm_type sa_alarm_type_disk = {
	.code = "disk",
	.init_alarm_config = disk_init_alarm_config,
	.parse_config_option = disk_parse_config_option,
	.check_config = disk_check_config,
	.check_alarm = disk_check_alarm,
};
