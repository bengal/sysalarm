/*
 * util.h
 *
 */

#ifndef UTIL_H_
#define UTIL_H_

#define SA_LOG_DEBUG 	0
#define SA_LOG_INFO 	1
#define SA_LOG_NOTICE 	2
#define SA_LOG_WARN 	3
#define SA_LOG_ERR 		4

void die(char *fmt, ...);
void sa_log(int level, char *fmt, ...);
void set_result(struct result *result, int code, char *fmt, ...);
int connect_tcp(char *host, unsigned short port);

#define CHECK_MALLOC(ptr)      if(!(ptr)) { \
	die("Allocation problem at %s:%d", __FILE__, __LINE__); \
}

extern int loglevel;

#endif /* UTIL_H_ */
