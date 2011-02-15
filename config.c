
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>

#include "config.h"
#include "util.h"

struct global_config global_config;
struct alarm_condition *current_alarm = NULL;
int default_section;

struct alarm_condition alarms[MAX_ALARM_NUM];

/*
 * ALARM TYPES
 */
extern struct alarm_type sa_alarm_type_disk;
struct alarm_type eof_type = {
	.code = NULL
};

struct alarm_type *alarm_types[] = {

		&sa_alarm_type_disk,

		/* array terminator */
		&eof_type
};
/*
 * ALARM TYPES END
 */


char *parse_section_name(char *str)
{
	char *name = strtok(str, " \t");
	if(name != NULL){
		char *delim = strtok(NULL, " \n");
		if(delim != NULL && strcmp(delim, "{") == 0){
			return strdup(name);
		}
	}

	printf("Fatal: expected start of section\n");
	exit(1);
}

int is_blank_line(char *line)
{
	char *ptr = line;
	while(*ptr != 0 && *ptr != '\n'){

		if(*ptr == '#')
			return 1;

		if(!isspace(*ptr))
			return 0;

		ptr++;
	}
	return 1;
}

int is_end_section(char * str)
{
	char *ptr = str;

	while(isspace(*ptr))
		ptr++;

	if(*ptr != '}')
		return 0;

	ptr++;
	while(isspace(*ptr))
		ptr++;

	if(*ptr == '\n' || *ptr == 0)
		return 1;

	return 0;
}

char *trim(char *str)
{
	char *ptr = str + strlen(str) - 1;
	while(*ptr == ' ' || *ptr == '\t' || *ptr == '\n') {
		*ptr = 0;
		ptr--;
	}

	ptr = str;
	while(*ptr != 0 && (*ptr == ' ' || *ptr == '\t')){
		ptr++;
	}

	return ptr;
}

void set_config_parameter(char *key, char *value)
{

	char *ptr = value + strlen(value) - 1;
	while(*ptr == ' ' || *ptr == '\t' || *ptr == '\n') {
		*ptr = 0;
		ptr--;
	}

	ptr = value;
	while(*ptr != 0 && (*ptr == ' ' || *ptr == '\t')){
		ptr++;
	}

	value = ptr;

	printf("setting config parameter %s:%s\n", key, value);
}


struct alarm_type *search_alarm_type(char *type)
{
	int i = 0;

	while(1){
		char *code = alarm_types[i]->code;

		if(code == NULL)
			break;

		if(!strcmp(code, type))
			return alarm_types[i];
	}

	return NULL;
}

void insert_alarm(struct alarm_condition *alarm)
{
	int i;

	for(i = 0; i < MAX_ALARM_NUM; i++){
		if(alarms[i].type == NULL){
			memcpy(&alarm[i], alarms, sizeof(struct alarm_condition));
			return;
		}
	}

	die("You can't define more than %d alarms!\n", MAX_ALARM_NUM);
}

void set_default_property(char *key, char *value)
{
	if(key == NULL)
		return;

	if(!strcmp(key, "")){

	}
}

void set_alarm_property(char *key, char *value)
{
	if(key == NULL)
		return;

	if(!strcmp(key, "type")){
		struct alarm_type *type = search_alarm_type(value);

		if(type == NULL){
			die("Error, type %s not found\n", value);
		}

		current_alarm->type = type;
		current_alarm->type->init_alarm_config(current_alarm);
		return;
	} else if(!strcmp(key, "notification")){

		if(!strcmp(value, "email")){
			current_alarm->notify_method = NOTIFY_METHOD_EMAIL;
		} else if(!strcmp(value, "beep")){
			current_alarm->notify_method = NOTIFY_METHOD_BEEP;
		} else {
			die("Unknown notification method '%s'", value);
		}

		return;
	} else if(!strcmp(key, "name")){
		current_alarm->name = strdup(value);
		return;
	}

	if(current_alarm->type == NULL){
		die("type should be defined before any alarm-specific property in section\n");
	}

	if(current_alarm->type->parse_config_option(current_alarm, key, value) == -1){
		die("Invalid parameter '%s' for alarm of type '%s'", key, current_alarm->type->code);
	}
}



void parse_config_file(char *file_name)
{
	FILE *file;
	char *line;
	ssize_t read, len;
	int line_num = 0;

	memset(alarms, 0, sizeof(struct alarm_condition) * 100);
	memset(&global_config, 0, sizeof(struct global_config));
	default_section = 0;

	debug("Parsing file %s\n", file_name);

	if((file = fopen(file_name, "r")) == NULL){
		die("Error opening configuration file %s\n", file_name);
	}

	line = malloc(BUF_LEN);

	while((read = getline(&line, &len, file)) != -1){

		line_num++;

		if(is_blank_line(line))
			continue;

		if(current_alarm == NULL){
			char *section_name = parse_section_name(line);
			if(strcmp(section_name, "alarm") == 0){
				current_alarm = malloc(sizeof(struct alarm_condition));
			} else if(!strcmp(section_name, "default")){
				default_section = 1;
			} else {
				die("Unknow section type '%s'", section_name);
			}

			continue;
		}

		if(is_end_section(line)){

			if(default_section){
				default_section = 0;
				continue;
			}

			if(current_alarm->type == NULL){
				die("You must specify an alarm type for every alarm section");
			}

			current_alarm->type->check_config(current_alarm);
			insert_alarm(current_alarm);
			free(current_alarm); // TODO static
			current_alarm = NULL;

			continue;
		}

		char *sep = strchr(line, '=');
		if(sep == NULL){
			die("Syntax error in %s, line %d\n", file_name, line_num);
		}

		char *key = trim(strtok(line, "="));
		char *value = trim(strtok(NULL, ""));

		if(default_section){
			set_default_property(key, value);
		} else {
			set_alarm_property(key, value);
		}



	}

}






