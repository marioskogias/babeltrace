#ifndef _BABELTRACE_SCRIBE_TYPES_H
#define _BABELTRACE_SCRIBE_TYPES_H

/*
 * Scribe output format (Text Output)
 *
 * Type header
 */

#include <sys/mman.h>
#include <errno.h>
#include <stdint.h>
#include <unistd.h>
#include <glib.h>
#include <babeltrace/babeltrace-internal.h>
#include <babeltrace/types.h>
#include <babeltrace/format.h>
#include <babeltrace/format-internal.h>
#include <scribe-client/client.h>
#include <babeltrace/scribe/raw-scribe-types.h>

/*
 * Define type enumeration
 */

enum scribe_output {
    TYPE_RAW,
    TYPE_ZIPKIN
};

typedef int (*formatter)(struct bt_stream_pos *pos,
			   struct ctf_stream_definition *stream);

static formatter *type_formatters[2] = {
    raw_scribe_formatters,
    NULL
    //zipkin_formatters
};

static rw_dispatch *rw_tables[2] = {
    raw_scribe_rw,
    NULL
    //zipkin_rw
}; 
/*
 *
 * Inherit from both struct bt_stream_pos and struct bt_trace_descriptor.
 */
struct scribe_stream_pos {
	struct bt_stream_pos parent;
	struct bt_trace_descriptor trace_descriptor;
	struct scribe_client *client ;		/* Scribe client. NULL if unset. */
	const char hostname[NAME_MAX];
    int port;
    char *log_event;
    int log_count;
    formatter *formatters_table;
    int depth;
	int dummy;		/* disable output */
	int print_names;	/* print field names */
	int field_nr;
	uint64_t last_real_timestamp;	/* to print delta */
	uint64_t last_cycles_timestamp;	/* to print delta */
	GString *string;	/* current string */
};


static
struct bt_trace_descriptor *scribe_generic_open_trace(const char *path, 
        int flags, void (*packet_seek)(struct bt_stream_pos *pos, size_t index,
			int whence), FILE *metadata_fp, enum scribe_output type);

static
struct bt_trace_descriptor *scribe_generic_close_trace(const char *path, 
        int flags, void (*packet_seek)(struct bt_stream_pos *pos, size_t index,
			int whence), FILE *metadata_fp);

#endif /* _BABELTRACE_SCRIBE_TYPES_H */
