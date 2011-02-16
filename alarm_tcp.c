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

struct tcp_alarm_options {
	char *host;
	int port;
};

static int tcp_check_alarm(struct alarm_condition *config)
{
	struct tcp_alarm_options *opt = SPECIFIC_CONFIG(opt, config);
	int sockfd;
	struct hostent *server;
	struct sockaddr_in serv_addr;

	debug("Connecting to %s %hd\n", opt->host, opt->port);

	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		return ALARM_ERROR;
	}

    if ((server = gethostbyname(opt->host)) == NULL) {
    	return ALARM_ON;
    }

    memset(&serv_addr, 0, sizeof(struct sockaddr_in));
    serv_addr.sin_family = AF_INET;
    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);
    serv_addr.sin_port = htons(opt->port);

    if(connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
    	return ALARM_ON;
    }

    shutdown(sockfd, SHUT_RDWR);

	return ALARM_OFF;
}



static int tcp_parse_config_option(struct alarm_condition *config, char *key, char *value)
{
	struct tcp_alarm_options *opt = SPECIFIC_CONFIG(opt, config);

	if(!strcmp(key, "host")){
		opt->host = strdup(value);
		return 0;
	} else if(!strcmp(key, "port")){
		opt->port = atoi(value); /* TODO error checking */
		return 0;
	}

	return -1;
}

static void tcp_check_config(struct alarm_condition *config)
{
	struct tcp_alarm_options *opt = SPECIFIC_CONFIG(opt, config);

	if(opt->host == NULL){
		printf("Alarm %s (type %s) requires parameter 'host'\n", config->name, config->type->code );
		exit(1);
	}

	if(opt->port < 0 || opt->port > 65535){
		printf("Alarm %s (type %s) requires parameter 'port'\n", config->name, config->type->code );
		exit(1);
	}

}

static void tcp_init_alarm_config(struct alarm_condition *config)
{
	struct tcp_alarm_options *opt = malloc(sizeof(struct tcp_alarm_options));
	config->specific_config = opt;
	memset(config->specific_config, 0, sizeof(struct tcp_alarm_options));
	opt->port = -1;
}


struct alarm_type sa_alarm_type_tcp = {
	.code = "tcp",
	.init_alarm_config = tcp_init_alarm_config,
	.parse_config_option = tcp_parse_config_option,
	.check_config = tcp_check_config,
	.check_alarm = tcp_check_alarm,
};
