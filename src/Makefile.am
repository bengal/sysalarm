bin_PROGRAMS = sysalarm

AM_CFLAGS = -Wall -O2 -DSYS_CONF_DIR='"$(sysconfdir)"' \
            -DLOCAL_STATE_DIR='"$(localstatedir)"'
AM_LDFLAGS = 

sysalarm_SOURCES = util.c base.c parse.c state.c \
	condition_disk.c condition_tcp.c condition_cmd.c \
	action_mail.c action_cmd.c action_xmpp.c \
	base.h parse.h util.h state.h \
	sysalarm.c  



