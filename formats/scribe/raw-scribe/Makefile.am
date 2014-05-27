AM_CFLAGS = $(PACKAGE_CFLAGS) -I$(top_srcdir)/include

SUBDIRS = types .

lib_LTLIBRARIES = libbabeltrace-raw-scribe.la

libbabeltrace_raw_scribe_la_SOURCES = \
	raw_scribe.c

libbabeltrace_raw_scribe_la_LDFLAGS = \
	-Wl,--no-as-needed  -version-info $(BABELTRACE_LIBRARY_VERSION) 

libbabeltrace_raw_scribe_la_LIBADD = \
	$(top_builddir)/lib/libbabeltrace.la \
	$(top_builddir)/formats/ctf/libbabeltrace-ctf.la \
	$(top_builddir)/formats/scribe/libbabeltrace-scribe.la \
	$(top_builddir)/formats/scribe/raw-scribe/types/libraw-scribe-types.la
