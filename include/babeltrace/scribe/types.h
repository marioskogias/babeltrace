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
#include <babeltrace/scribe/client.h>

/*
 * Inherit from both struct bt_stream_pos and struct bt_trace_descriptor.
 */
struct scribe_stream_pos {
	struct bt_stream_pos parent;
	struct bt_trace_descriptor trace_descriptor;
	struct scribe_client* client ;		/* Scribe client. NULL if unset. */
	int depth;
	int dummy;		/* disable output */
	int print_names;	/* print field names */
	int field_nr;
	uint64_t last_real_timestamp;	/* to print delta */
	uint64_t last_cycles_timestamp;	/* to print delta */
	GString *string;	/* Current string */
};


#endif /* _BABELTRACE_SCRIBE_TYPES_H */
