/*
 * alarm_tcp.c
 */

#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>

#include "base.h"
#include "util.h"

struct tcp_condition_config {
	char *tcp_host;
	int tcp_port;
};

static int tcp_cond_set_options(struct condition *condition, struct option_value *options)
{
	struct option_value *option;

	struct tcp_condition_config *config = calloc(1, sizeof(struct tcp_condition_config));
	condition->priv_config = config;

	CHECK_MALLOC(config);

	for (option = options; option != NULL; option = option->next) {

		if (!strcmp(option->name, "tcp_host")) {
			config->tcp_host = strdup(option->value);
		} else if (!strcmp(option->name, "tcp_port")) {
			config->tcp_port = atoi(option->value);
		} else {
			die("Unknown option '%s' for condition '%s'", option->name,
			    condition->name);
		}
	}

	if(config->tcp_host == NULL || config->tcp_port == 0)
		die("Tcp condition: you must supply both 'tcp_host' and 'tcp_port' parameters");

	return 0;
}

static void tcp_cond_check_condition(struct condition *condition, struct result *result)
{
	struct tcp_condition_config *config = condition->priv_config;
	int sockfd;

	if((sockfd = connect_tcp(config->tcp_host, config->tcp_port)) >= 0){
		shutdown(sockfd, SHUT_RDWR);
		set_result(result, CONDITION_OFF, NULL);
		return;;
	}

	set_result(result, CONDITION_ON, "TCP port %d unreachable on host '%s'",
			config->tcp_port, config->tcp_host);
}

struct condition_type condition_type_tcp = {
	.name = "TCP",
	.description = "Tries to establish a tcp connection to a given host",
	.set_options = tcp_cond_set_options,
	.check_condition = tcp_cond_check_condition,
};
