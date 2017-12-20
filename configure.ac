AC_PREREQ([2.69])
AC_INIT([cchat], [1.0])
AM_INIT_AUTOMAKE([foreign])
AC_CONFIG_SRCDIR([cchat.c])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.
AC_CHECK_LIB([ncurses], [initscr])

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL

# Checks for library functions.
AC_CHECK_FUNCS([strerror strstr])

CFLAGS="$CFLAGS -Wall -O2"
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
