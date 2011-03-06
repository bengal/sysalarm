bin_PROGRAMS = sysalarm

AM_CFLAGS = -Wall -O2 -DSYSCONFDIR='"$(sysconfdir)"'
AM_LDFLAGS = 

sysalarm_SOURCES = util.c base.c parse.c \
	condition_disk.c condition_tcp.c condition_cmd.c \
	action_mail.c action_cmd.c \
	base.h parse.h util.h \
	sysalarm.c  



