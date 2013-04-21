#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.67])
AC_INIT([cyberfeeder], [0], [jack@jackkelly.name])
AC_CONFIG_AUX_DIR([auxfiles])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
dnl AC_CONFIG_SRCDIR([])
AM_INIT_AUTOMAKE([1.11 foreign -Wall -Werror subdir-objects])
AM_SILENT_RULES([yes])

# Checks for programs.
AC_PROG_CC_C99

# Checks for libraries.
PKG_CHECK_MODULES([GTK3], [gtk+-3.0])

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.
AM_PROG_CC_C_O

# Checks for library functions.

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
