AM_CFLAGS = $(PACKAGE_CFLAGS) -I$(top_srcdir)/include

noinst_LTLIBRARIES = libraw-scribe-types.la

libraw_scribe_types_la_SOURCES = \
	integer.c \
    float.c \
	enum.c \
	string.c \
	struct.c \
	variant.c \
	array.c \
	sequence.c
 
libraw_scribe_types_la_LIBADD = \
	$(top_builddir)/lib/libbabeltrace.la
