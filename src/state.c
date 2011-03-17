/*
 * state.c
 *
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "base.h"
#include "state.h"
#include "util.h"

#ifndef LOCAL_STATE_DIR
#define LOCAL_STATE_DIR "/var"
#endif
#define STATE_FILE LOCAL_STATE_DIR "/run/sysalarm.state"

int read_cond_states()
{
	char file_name[] = STATE_FILE;
	FILE *file;
	char name[256];
	int first_true;
	int last_alarm;

	if((file = fopen(file_name, "r")) == NULL){
		return -1;
	}

	while (fscanf(file, "%255s %d %d", name, &first_true, &last_alarm ) != EOF) {
		struct condition * cond = search_condition(name);
		if (cond != NULL) {
			sa_log(SA_LOG_DEBUG, "state for '%s' : %d %d\n", name, first_true, last_alarm);
			cond->first_true = first_true;
			cond->last_alarm = last_alarm;
		} else {
			sa_log(SA_LOG_DEBUG, "condition %s not found reading state file\n", name);
		}
	}

	fclose(file);
	return 0;
}

int write_cond_states()
{
	char file_name[] = STATE_FILE;
	int fd;
	FILE *file;
	int i;

	if((fd = open(file_name, O_CREAT|O_WRONLY|O_TRUNC, 0600)) == -1){
		return -1;
	}

	/* TODO: lock */

	if((file = fdopen(fd, "w")) == NULL){
		close(fd);
		return -2;
	}

	for(i = 0; i < MAX_ELEMENTS; i++){
		struct condition *cond = &conditions[i];
		if(cond->name == NULL)
			break;
		fprintf(file, "%.255s %d %d\n", cond->name, (int)cond->first_true,
				(int)cond->last_alarm);
	}

	/* TODO: unlock */

	fclose(file);
	return 0;
}



