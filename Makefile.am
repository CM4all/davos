AUTOMAKE_OPTIONS = foreign 1.11 subdir-objects

noinst_LIBRARIES = libutil.a

libutil_a_SOURCES = \
	src/util/CharUtil.hxx \
	src/util/LightString.hxx \
	src/util/UriEscape.cxx src/util/UriEscape.hxx \
	src/gmtime.c src/gmtime.h \
	src/date.c src/date.h \
	src/format.c src/format.h
libutil_a_CPPFLAGS = $(AM_CPPFLAGS) \
	$(LIBCM4ALL_INLINE_CFLAGS)

wasdir = $(pkglibdir)/was-bin
was_PROGRAMS = src/davos-plain src/davos-od

src_davos_plain_SOURCES = \
	src/util.cxx src/util.hxx \
	src/uri_escape.cxx src/uri_escape.hxx \
	src/expat.hxx src/expat.cxx \
	src/wxml.cxx src/wxml.hxx \
	src/error.cxx src/error.hxx \
	src/splice.cxx src/splice.hxx \
	src/directory.cxx src/directory.hxx \
	src/get.cxx src/get.hxx \
	src/put.cxx src/put.hxx \
	src/propfind.cxx src/propfind.hxx \
	src/proppatch.cxx src/proppatch.hxx \
	src/lock.cxx src/lock.hxx \
	src/other.cxx src/other.hxx \
	src/file.cxx src/file.hxx \
	src/writer.hxx \
	src/frontend.hxx \
	src/main.cxx
src_davos_plain_CPPFLAGS = $(AM_CPPFLAGS) \
	$(EXPAT_CFLAGS) \
	$(LIBCM4ALL_INLINE_CFLAGS) \
	$(LIBFOX_CFLAGS) \
	$(LIBWAS_CFLAGS)
src_davos_plain_LDADD = \
	libutil.a \
	$(EXPAT_LIBS) \
	$(LIBFOX_LIBS) \
	$(LIBWAS_LIBS)

src_davos_od_SOURCES = \
	src/util.cxx src/util.hxx \
	src/mime_types.cxx src/mime_types.hxx \
	src/uri_escape.cxx src/uri_escape.hxx \
	src/expat.hxx src/expat.cxx \
	src/wxml.cxx src/wxml.hxx \
	src/splice.cxx src/splice.hxx \
	src/proppatch.cxx src/proppatch.hxx \
	src/lock.cxx src/lock.hxx \
	src/writer.hxx \
	src/frontend.hxx \
	src/od_resource.cxx src/od_resource.hxx \
	src/od_backend.hxx \
	src/od_put.cxx \
	src/od_main.cxx
src_davos_od_CPPFLAGS = $(AM_CPPFLAGS) \
	$(EXPAT_CFLAGS) \
	$(LIBCM4ALL_INLINE_CFLAGS) \
	$(LIBOD_CFLAGS) \
	$(LIBWAS_CFLAGS)
src_davos_od_LDADD = \
	libutil.a \
	$(EXPAT_LIBS) \
	$(LIBOD_LIBS) \
	$(LIBWAS_LIBS)

# unit tests

TESTS = test/t_uri_escape
noinst_PROGRAMS = test/t_uri_escape

test_t_uri_escape_SOURCES = test/t_uri_escape.cxx
test_t_uri_escape_CPPFLAGS = $(AM_CPPFLAGS) -I$(srcdir)/src \
	$(LIBCM4ALL_INLINE_CFLAGS)
test_t_uri_escape_LDADD = libutil.a

#
# documentation
#

MOSTLYCLEANFILES = doc/davos.html

if DOCUMENTATION
doc/davos.html: doc/davos.xml
	$(XMLTO) -o $(@D) html-nochunks $<

all-local: doc/davos.html

doc_DATA = doc/davos.html
endif
