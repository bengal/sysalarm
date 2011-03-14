# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.67])
AC_INIT([sysalarm], [0.1.0], [b.galvani@gmail.com])
AC_CONFIG_AUX_DIR(config)
AC_CONFIG_SRCDIR([src/util.c])
AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE
AM_SILENT_RULES([yes])

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL

# Checks for libraries.
AC_CHECK_LIB([esmtp], [smtp_create_session], [], [AC_MSG_ERROR([libesmtp not found])])

PKG_CHECK_MODULES(IKSEMEL,iksemel,,exit)


# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h netdb.h stdlib.h string.h sys/socket.h sys/vfs.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_CHECK_FUNCS([gethostbyname gethostname memset socket strchr strdup])


CFLAGS="$CFLAGS $IKSEMEL_CFLAGS"
LDFLAGS="$LDFLAGS $IKSEMEL_LIBS"


AC_CONFIG_FILES([Makefile
                 man/Makefile
                 src/Makefile])
AC_OUTPUT
