/*
 * alarm_tcp.c
 */

#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>

#include "config.h"
#include "util.h"

struct tcp_condition_config {
	char *host;
	int port;
};

static int tcp_cond_set_options(struct condition *condition, struct option_value *options)
{
	struct option_value *option;

	struct tcp_condition_config *config = malloc(sizeof(struct tcp_condition_config));
	condition->specific_config = config;

	for (option = options; option != NULL; option = option->next) {

		if(!option->specific)
			continue;

		if (!strcmp(option->name, "host")) {
			config->host = strdup(option->value);
		} else if (!strcmp(option->name, "port")) {
			config->port = atoi(option->value);
		} else {
			die("Unknown option '%s' for condition '%s'", option->name,
			    condition->name);
		}
	}

	if(config->host == NULL || config->port == 0)
		die("Tcp condition: you must supply both host and port parameters");

	return 0;
}

static int tcp_cond_check_condition(struct condition *condition)
{
	struct tcp_condition_config *config = condition->specific_config;
	int sockfd;

	if((sockfd = connect_tcp(config->host, config->port)) >= 0){
		shutdown(sockfd, SHUT_RDWR);
		return CONDITION_OFF;
	}

	return CONDITION_ON;
}


struct condition_type condition_type_tcp = {
	.name = "TCP",
	.set_options = tcp_cond_set_options,
	.check_condition = tcp_cond_check_condition,
};
