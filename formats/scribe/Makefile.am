AM_CFLAGS = $(PACKAGE_CFLAGS) -I$(top_srcdir)/include

SUBDIRS = . raw-scribe

noinst_LTLIBRARIES = libbabeltrace-scribe.la

libbabeltrace_scribe_la_SOURCES = \
	scribe.c

libbabeltrace_scribe_la_LDFLAGS = \
	-Wl,--no-as-needed -version-info $(BABELTRACE_LIBRARY_VERSION) \
	-lscribe_client 

libbabeltrace_scribe_la_LIBADD = \
	$(top_builddir)/lib/libbabeltrace.la
	$(top_builddir)/formats/ctf/libbabeltrace-ctf.la
