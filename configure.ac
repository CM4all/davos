AC_PREREQ(2.60)
AC_INIT(CM4all Davos, 0.7, mk@cm4all.com, cm4all-davos)
AC_COPYRIGHT(Copyright 2013-2015 Content Management AG)
AC_CONFIG_SRCDIR(src/main.cxx)
AC_CONFIG_AUX_DIR(build)
AM_INIT_AUTOMAKE([foreign 1.11 subdir-objects silent-rules])

dnl Checks for programs.
AC_PROG_INSTALL
AC_PROG_CC_C99
AC_PROG_CXX
AC_PROG_RANLIB

dnl
dnl Checks for libraries.
dnl

PKG_CHECK_MODULES([EXPAT], [expat],,
	[AC_MSG_ERROR([expat not found])])

PKG_CHECK_MODULES([LIBCM4ALL_INLINE], [libcm4all-inline >= 0.3.3],,
	[AC_MSG_ERROR([libcm4all-inline not found])])

PKG_CHECK_MODULES([LIBWAS], [libcm4all-was-simple >= 1.0],,
	[AC_MSG_ERROR([libwas not found])])

PKG_CHECK_MODULES([LIBFOX], [libcm4all-fox >= 0.2.1],,
	[AC_MSG_ERROR([libfox not found])])

PKG_CHECK_MODULES([LIBOD], [libcm4all-od-setup >= 1.2],,
	[AC_MSG_ERROR([libod not found])])

dnl work around compiler warnings in GLib headers
LIBWAS_CFLAGS=$(echo ${LIBWAS_CFLAGS} |sed -e 's,-I,-isystem ,g')
LIBOD_CFLAGS=$(echo ${LIBOD_CFLAGS} |sed -e 's,-I,-isystem ,g')

dnl
dnl CFLAGS
dnl

AC_SUBST(AM_CFLAGS)
AC_SUBST(AM_CXXFLAGS)
AC_SUBST(AM_CPPFLAGS)
AC_SUBST(AM_LDFLAGS)

AM_CFLAGS=""
AM_CXXFLAGS="-std=c++11"
AM_CPPFLAGS="-D_REENTRANT -D_GNU_SOURCE"
AM_LDFLAGS=""

if test "$am_cv_CC_dependencies_compiler_type" = "gcc3"; then
	AM_CFLAGS="$AM_CFLAGS -Wall -Wextra -Wmissing-prototypes -Wwrite-strings -Wcast-qual -Wfloat-equal -Wshadow -Wpointer-arith -Wbad-function-cast -Wsign-compare -Waggregate-return -Wmissing-declarations -Wmissing-noreturn -Wmissing-format-attribute -Wredundant-decls -Wnested-externs -Wno-long-long -Wstrict-prototypes -Wundef -pedantic"

	AM_CXXFLAGS="$AM_CXXFLAGS -Wall -Wextra"
	AM_CXXFLAGS="$AM_CXXFLAGS -Wwrite-strings -Wcast-qual -Wcast-align -Wfloat-equal"
	AM_CXXFLAGS="$AM_CXXFLAGS -Wshadow -Wpointer-arith -Wsign-compare"
	AM_CXXFLAGS="$AM_CXXFLAGS -Wmissing-declarations -Wmissing-noreturn -Wmissing-format-attribute"
	AM_CXXFLAGS="$AM_CXXFLAGS -Wredundant-decls -Wno-long-long -Wundef"
	AM_CXXFLAGS="$AM_CXXFLAGS -Wno-variadic-macros"
	AM_CXXFLAGS="$AM_CXXFLAGS"
fi

dnl
dnl DEBUG
dnl

AC_ARG_ENABLE(debug,
	AS_HELP_STRING([--disable-debug],
		[Disable debugging (default: enabled)]),,
	enable_debug=yes)

if test x$enable_debug = xno; then
	AM_CFLAGS="$AM_CFLAGS -ffunction-sections -fdata-sections"
	AM_CXXFLAGS="$AM_CXXFLAGS -ffunction-sections -fdata-sections"
	AM_CPPFLAGS="$AM_CPPFLAGS -DNDEBUG"
	AM_LDFLAGS="$AM_LDFLAGS -Wl,-gc-sections"
fi

AC_ARG_ENABLE(werror,
	AS_HELP_STRING([--enable-werror],
		[treat warnings as errors]),,
	enable_werror=$enable_debug)

if test x$enable_werror = xyes; then
	AM_CFLAGS="$AM_CFLAGS -Werror -pedantic-errors"
	AM_CXXFLAGS="$AM_CXXFLAGS -Werror"
fi

dnl
dnl Documentation
dnl

AC_ARG_ENABLE(documentation,
	AS_HELP_STRING([--disable-documentation],
		[Disable documentation (default: enabled)]),,
	enable_documentation=yes)
AM_CONDITIONAL(DOCUMENTATION, test x$enable_documentation = xyes)

if test x$enable_documentation = xyes; then
	AC_PATH_PROG([XMLTO], [xmlto], [no])
	if test x$XMLTO = xno; then
		AC_MSG_ERROR([xmlto not found])
	fi
fi

dnl
dnl Output files
dnl

AC_OUTPUT(Makefile)
