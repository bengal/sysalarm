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

static int tcp_cond_check_alarm(struct condition *condition)
{
	struct tcp_condition_config *config = condition->specific_config;
	int sockfd;
	struct hostent *server;
	struct sockaddr_in serv_addr;

	debug("Connecting to %s %hd\n", config->host, config->port);

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		return CONDITION_ERROR;
	}

	if ((server = gethostbyname(config->host)) == NULL) {
		return CONDITION_ON;
	}

	memset(&serv_addr, 0, sizeof(struct sockaddr_in));
	serv_addr.sin_family = AF_INET;
	memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);
	serv_addr.sin_port = htons(config->port);

	if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		return CONDITION_ON;
	}

	shutdown(sockfd, SHUT_RDWR);

	return CONDITION_OFF;
}


struct condition_type condition_type_tcp = {
	.name = "tcp",
	.set_options = tcp_cond_set_options,
	.check_condition = tcp_cond_check_alarm,
};
