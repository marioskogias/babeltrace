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

/*
 * Inherit from both struct bt_stream_pos and struct bt_trace_descriptor.
 */
struct scribe_stream_pos {
	struct bt_stream_pos parent;
	struct bt_trace_descriptor trace_descriptor;
	struct scribe_client* client ;		/* Scribe client. NULL if unset. */
	const char hostname[NAME_MAX];
    int port;
    char * log_event;
    int log_count;
    int depth;
	int dummy;		/* disable output */
	int print_names;	/* print field names */
	int field_nr;
	uint64_t last_real_timestamp;	/* to print delta */
	uint64_t last_cycles_timestamp;	/* to print delta */
	GString *string;	/* Current string */
};

/*
 * Write only is supported for now.
 */
BT_HIDDEN
int scribe_integer_write(struct bt_stream_pos *pos, struct bt_definition *definition);

BT_HIDDEN
int scribe_float_write(struct bt_stream_pos *pos, struct bt_definition *definition);

BT_HIDDEN
int scribe_enum_write(struct bt_stream_pos *pos, struct bt_definition *definition);

BT_HIDDEN
int scribe_string_write(struct bt_stream_pos *pos, struct bt_definition *definition);

BT_HIDDEN
int scribe_struct_write(struct bt_stream_pos *pos, struct bt_definition *definition);

BT_HIDDEN
int scribe_variant_write(struct bt_stream_pos *pos, struct bt_definition *definition);

BT_HIDDEN
int scribe_array_write(struct bt_stream_pos *pos, struct bt_definition *definition);

BT_HIDDEN
int scribe_sequence_write(struct bt_stream_pos *pos, struct bt_definition *definition);
/*
 * Check if the field must be printed.
 */
BT_HIDDEN
int print_field(struct bt_definition *definition);

#endif /* _BABELTRACE_SCRIBE_TYPES_H */
