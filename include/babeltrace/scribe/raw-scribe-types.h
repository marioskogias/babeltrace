#ifndef _BABELTRACE_SCRIBE_RAW_TYPES_H
#define _BABELTRACE_SCRIBE_RAW_TYPES_H

#include <babeltrace/scribe/types.h>

static
rw_dispatch raw_scribe_rw[] = {
	[ CTF_TYPE_INTEGER ] = scribe_raw_integer_write,
	[ CTF_TYPE_FLOAT ] = scribe_raw_float_write,
	[ CTF_TYPE_ENUM ] = scribe_raw_enum_write,
	[ CTF_TYPE_STRING ] = scribe_raw_string_write,
	[ CTF_TYPE_STRUCT ] = scribe_raw_struct_write,
	[ CTF_TYPE_VARIANT ] = scribe_raw_variant_write,
	[ CTF_TYPE_ARRAY ] = scribe_raw_array_write,
	[ CTF_TYPE_SEQUENCE ] = scribe_raw_sequence_write,
};

/*
 * Formatters
 */

int raw_format_pre_payload(struct bt_stream_pos *pos, 
        struct ctf_stream_definition *stream);

int raw_format_payload(struct bt_stream_pos *pos, 
        struct ctf_stream_definition *stream);

formatter raw_scribe_formatters[] = {
    raw_format_pre_payload,
    raw_format_payload
};

/*
* Write only is supported for now.
*/
BT_HIDDEN
int scribe_raw_integer_write(struct bt_stream_pos *pos, 
        struct bt_definition *definition);

BT_HIDDEN
int scribe_raw_float_write(struct bt_stream_pos *pos, 
        struct bt_definition *definition);

BT_HIDDEN
int scribe_raw_enum_write(struct bt_stream_pos *pos, 
        struct bt_definition *definition);

BT_HIDDEN
int scribe_raw_string_write(struct bt_stream_pos *pos, 
        struct bt_definition *definition);

BT_HIDDEN
int scribe_raw_struct_write(struct bt_stream_pos *pos, 
        struct bt_definition *definition);

BT_HIDDEN
int scribe_raw_variant_write(struct bt_stream_pos *pos, 
        struct bt_definition *definition);

BT_HIDDEN
int scribe_raw_array_write(struct bt_stream_pos *pos, 
        struct bt_definition *definition);

BT_HIDDEN
int scribe_raw_sequence_write(struct bt_stream_pos *pos, 
        struct bt_definition *definition);
/*
 * Check if the field must be printed.
 */
BT_HIDDEN
int print_field(struct bt_definition *definition);
#endif /* _BABELTRACE_SCRIBE_RAW_TYPES_H */
