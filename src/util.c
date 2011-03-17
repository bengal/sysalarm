/*
 * util.c
 *
 */

#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <syslog.h>

#include "base.h"
#include "util.h"


int loglevel = SA_LOG_WARN;
int logopen = 0;

void die(char *fmt, ...)
{

	va_list argList;

	printf("FATAL ERROR:\n");
	va_start(argList, fmt);
	vprintf(fmt, argList);
	va_end(argList);
	printf("\n");

	exit(1);
}

void sa_log(int level, char *fmt, ...)
{
	va_list argList;

	if(level >= loglevel){
		if(!logopen){
			openlog("sysalarm", 0, LOG_USER);
			logopen = 1;
		}

		va_start(argList, fmt);
		vsyslog(LOG_NOTICE, fmt, argList);
		va_end(argList);
	}
}


void set_result(struct result *result, int code, char *fmt, ...)
{
	va_list arg_list;
	result->code = code;
	va_start(arg_list, fmt);
	vsnprintf(result->desc, RESULT_DESC_LEN, fmt, arg_list);
	va_end(arg_list);
}

int connect_tcp(char *host, unsigned short port)
{
	int sockfd;
	struct hostent *server;
	struct sockaddr_in serv_addr;

	sa_log(SA_LOG_DEBUG, "Connecting to %s %hd\n", host, port);

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		return -1;
	}

	if ((server = gethostbyname(host)) == NULL) {
		return -1;
	}

	memset(&serv_addr, 0, sizeof(struct sockaddr_in));
	serv_addr.sin_family = AF_INET;
	memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);
	serv_addr.sin_port = htons(port);

	if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		return -1;
	}

	return sockfd;
}
