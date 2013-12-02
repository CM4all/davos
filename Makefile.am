AUTOMAKE_OPTIONS = foreign 1.11 subdir-objects

wasdir = $(pkglibdir)/was-bin
was_PROGRAMS = src/davos

src_davos_SOURCES = \
	src/gmtime.c src/gmtime.h \
	src/date.c src/date.h \
	src/format.c src/format.h \
	src/wxml.hxx \
	src/error.cxx src/error.hxx \
	src/file.cxx src/file.hxx \
	src/put.cxx src/put.hxx \
	src/propfind.cxx src/propfind.hxx \
	src/main.cxx
src_davos_CPPFLAGS = $(AM_CPPFLAGS) $(LIBCM4ALL_INLINE_CFLAGS) $(LIBWAS_CFLAGS)
src_davos_LDADD = $(LIBWAS_LIBS)
