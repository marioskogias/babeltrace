#ifndef _BABELTRACE_SCRIBE_INTERNAL_H
#define _BABELTRACE_SCRIBE_INTERNAL_H

#include <babeltrace/types.h>
#include <babeltrace/format-internal.h>

enum scribe_output {
    TYPE_RAW,
    TYPE_ZIPKIN
};

typedef int (*formatter)(struct bt_stream_pos *pos,
			   struct ctf_stream_definition *stream);

#endif /* _BABELTRACE_SCRIBE_INTERNAL_H */
