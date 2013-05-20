#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.67])
AC_INIT([Cyberfeeder], [0], [jack@jackkelly.name])
AC_CONFIG_AUX_DIR([auxfiles])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([src/cyberfeeder.c])
AM_INIT_AUTOMAKE([1.11 foreign -Wall -Werror subdir-objects])
AM_SILENT_RULES([yes])

# Checks for programs.
AC_PROG_CC_C99

AC_ARG_VAR([GPERF], [Path to the gperf perfect hash function generator.])
AC_PATH_PROG([GPERF], [gperf], [no])
AS_IF([test "$GPERF" = no], [AC_MSG_ERROR([Cannot find gperf.])])

# Checks for libraries.
PKG_CHECK_MODULES([GTK3], [gtk+-3.0])
PKG_CHECK_MODULES([Jansson], [jansson])

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.
AM_PROG_CC_C_O

# Checks for library functions.

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
