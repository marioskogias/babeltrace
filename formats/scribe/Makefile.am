AM_CFLAGS = $(PACKAGE_CFLAGS) -I$(top_srcdir)/include

lib_LTLIBRARIES = libbabeltrace-scribe.la

libbabeltrace_scribe_la_SOURCES = \
	scribe.c

# Request that the linker keeps all static libraries objects.
libbabeltrace_scribe_la_LDFLAGS = \
	-Wl,--no-as-needed -version-info $(BABELTRACE_LIBRARY_VERSION) \
	-lscribe_client

libbabeltrace_scribe_la_LIBADD = \
	$(top_builddir)/lib/libbabeltrace.la
