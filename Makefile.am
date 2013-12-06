AUTOMAKE_OPTIONS = foreign 1.11 subdir-objects

wasdir = $(pkglibdir)/was-bin
was_PROGRAMS = src/davos

src_davos_SOURCES = \
	src/gmtime.c src/gmtime.h \
	src/date.c src/date.h \
	src/format.c src/format.h \
	src/expat.hxx src/expat.cxx \
	src/wxml.hxx \
	src/error.cxx src/error.hxx \
	src/directory.cxx src/directory.hxx \
	src/file.cxx src/file.hxx \
	src/put.cxx src/put.hxx \
	src/propfind.cxx src/propfind.hxx \
	src/proppatch.cxx src/proppatch.hxx \
	src/lock.cxx src/lock.hxx \
	src/other.cxx src/other.hxx \
	src/main.cxx
src_davos_CPPFLAGS = $(AM_CPPFLAGS) \
	$(EXPAT_CFLAGS) \
	$(LIBCM4ALL_INLINE_CFLAGS) \
	$(LIBFOX_CFLAGS) \
	$(LIBWAS_CFLAGS)
src_davos_LDADD = \
	$(EXPAT_LIBS) \
	$(LIBFOX_LIBS) \
	$(LIBWAS_LIBS)
