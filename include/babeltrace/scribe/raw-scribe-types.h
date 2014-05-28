#ifndef _BABELTRACE_SCRIBE_RAW_TYPES_H
#define _BABELTRACE_SCRIBE_RAW_TYPES_H

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
