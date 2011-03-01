/*
 * util.h
 *
 */

#ifndef UTIL_H_
#define UTIL_H_

void die(char *fmt, ...);
void debug(char *fmt, ...);
int connect_tcp(char *host, unsigned short port);

#define CHECK_MALLOC(ptr)      if(!(ptr)) { \
	die("Allocation problem at %s:%d", __FILE__, __LINE__); \
}

#endif /* UTIL_H_ */
