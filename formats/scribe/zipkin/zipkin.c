/*
 * BabelTrace - Scribe Output
 */

#include <babeltrace/scribe/types.h>
#include <babeltrace/format.h>
#include <babeltrace/babeltrace-internal.h>
#include <babeltrace/ctf/events-internal.h>
#include <inttypes.h>
#include <sys/mman.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <glib.h>
#include <unistd.h>
#include <stdlib.h>

#define NSEC_PER_SEC 1000000000ULL

static
struct bt_trace_descriptor *zipkin_open_trace(const char *path, int flags,
		void (*packet_seek)(struct bt_stream_pos *pos, size_t index,
			int whence), FILE *metadata_fp);


static
struct bt_format zipkin_format = {
	.open_trace = zipkin_open_trace,
	.close_trace = scribe_generic_close_trace,
};

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

static
void __attribute__((constructor)) zipkin_init(void)
{
	int ret;

	zipkin_format.name = g_quark_from_static_string("zipkin");
	ret = bt_register_format(&zipkin_format);
	assert(!ret);
}

static
void __attribute__((destructor)) zipkin_exit(void)
{
	bt_unregister_format(&zipkin_format);
}

static
struct bt_trace_descriptor *zipkin_open_trace(const char *path, int flags,
		void (*packet_seek)(struct bt_stream_pos *pos, size_t index,
			int whence), FILE *metadata_fp)
{
    struct bt_trace_descriptor *ret;
    
    ret = scribe_generic_open_trace(path, flags, NULL, metadata_fp);
    
    struct scribe_stream_pos *ppos = container_of(ret, struct scribe_stream_pos,
            trace_descriptor);
    
    /* Set formatters table */
    ppos->formatters_table = zipkin_formatters;
    /* Set rw table */
    ppos->parent.rw_table = NULL;
    
    return ret;
}

int zipkin_format_pre_payload(struct bt_stream_pos *ppos, 
        struct ctf_stream_definition *stream)
{
    return 1;
}

int zipkin_format_payload(struct bt_stream_pos *ppos, 
        struct ctf_stream_definition *stream)
{
    
    /*
     * To be implemented
     */
	return 1;
}
