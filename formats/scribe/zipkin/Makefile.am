AM_CFLAGS = $(PACKAGE_CFLAGS) -I$(top_srcdir)/include

SUBDIRS = .

lib_LTLIBRARIES = libbabeltrace-zipkin.la

libbabeltrace_zipkin_la_SOURCES = \
	zipkin.c

libbabeltrace_zipkin_la_LDFLAGS = \
	-Wl,--no-as-needed  -version-info $(BABELTRACE_LIBRARY_VERSION) 

libbabeltrace_zipkin_la_LIBADD = \
	$(top_builddir)/lib/libbabeltrace.la \
	$(top_builddir)/formats/ctf/libbabeltrace-ctf.la \
	$(top_builddir)/formats/scribe/libbabeltrace-scribe.la
