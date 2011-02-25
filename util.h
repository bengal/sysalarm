/*
 * util.h
 *
 */

#ifndef UTIL_H_
#define UTIL_H_

void die(char *fmt, ...);
void debug(char *fmt, ...);
int connect_tcp(char *host, unsigned short port);

#endif /* UTIL_H_ */
