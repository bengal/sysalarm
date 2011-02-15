/*
 * alarm_disk.c
 *
 */

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/vfs.h>

#define CAST_DISK_CONFIG(cfg,dest) struct disk_alarm_options *dest = (struct disk_alarm_options *)cfg->specific_config

struct disk_alarm_options {
	char *device;
	int threshold;
};


static int disk_check_alarm(struct alarm_condition *config)
{
	CAST_DISK_CONFIG(config, opt);
	struct statfs stat;

	if(statfs(opt->device, &stat) == -1)
		return -1;

	long disk_usage = stat.f_bfree * 100 / stat.f_blocks;
	printf("Disk usage for %s : %ld / %ld = %ld\n", opt->device, stat.f_bfree, stat.f_blocks, disk_usage);

	unsigned int usage = disk_usage;
	if(usage >= opt->threshold)
		return 1;

	return 0;
}



static void disk_parse_config_option(struct alarm_condition *config, char *key, char *value)
{
	CAST_DISK_CONFIG(config, opt);

	if(!strcmp(key, "device")){
		opt->device = strdup(value);
	} else if(!strcmp(key, "threshold")){
		opt->threshold = atoi(value);
	} else {
		printf("Fatal. Unknown argument %s for alarm of type disk", key);
		exit(1);
	}
}

static void disk_check_config(struct alarm_condition *config)
{
	CAST_DISK_CONFIG(config, opt);
	//struct disk_alarm_options *opt = (struct disk_alarm_options *)config;

	if(opt->device == NULL){
		printf("Alarm %s (type %s) requires parameter 'device'\n", config->name, config->type->code );
		exit(1);
	}

	if(opt->threshold < 1 || opt->threshold > 99){
		printf("Alarm %s (type %s) requires parameter 'threshold'\n", config->name, config->type->code );
		exit(1);
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




