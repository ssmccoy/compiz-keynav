AC_PREREQ(2.57)

AC_INIT([keynav], [0.0.1], [Scott S. McCoy])

AC_CONFIG_AUX_DIR(config)

AM_INIT_AUTOMAKE([dist-bzip2])
AC_CONFIG_HEADER([config.h])
AM_MAINTAINER_MODE

AC_ISC_POSIX
AC_PROG_CC
AC_PROG_LIBTOOL
AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h])

AC_CHECK_PROGS(pkg_config_cmd, pkg-config)
if test "x$pkg_config_cmd" = "x"; then
  AC_MSG_ERROR([** ERROR: pkg-config is required.])
fi

PKG_CHECK_MODULES(COMPIZ, compiz)

plugindir=`pkg-config --variable=libdir`/compiz
AC_SUBST(plugindir)

AC_OUTPUT([Makefile])
