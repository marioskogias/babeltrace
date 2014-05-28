#ifndef _BABELTRACE_ZIPKIN_TYPES_H
#define _BABELTRACE_ZIPKIN_TYPES_H

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
#include <babeltrace/scribe/scribe-internal.h>


/*
 * Formatters
 */

int zipkin_format_pre_payload(struct bt_stream_pos *pos, 
        struct ctf_stream_definition *stream);

int zipkin_format_payload(struct bt_stream_pos *pos, 
        struct ctf_stream_definition *stream);

static formatter zipkin_formatters[] = {
    zipkin_format_pre_payload,
    zipkin_format_payload
};
#endif /* _BABELTRACE_ZIPKIN_TYPES_H */
