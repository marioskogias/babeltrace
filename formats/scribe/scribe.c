/*
 * BabelTrace - Scribe Output
 */

#include <babeltrace/ctf-text/types.h>
#include <babeltrace/scribe/types.h>
#include <babeltrace/format.h>
#include <babeltrace/babeltrace-internal.h>
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

static
int scribe_write_event(struct bt_stream_pos *ppos, struct ctf_stream_definition *stream)
{
	return 0;
}


int parse_url(const char * path, struct scribe_stream_pos * scribe_stream)
{
	printf("The path from the function is %s\n", path);
    int ret;
    ret = sscanf(path, "scribe://%[a-zA-Z.0-9%-]:%d", scribe_stream->hostname, 
                &scribe_stream->port);
    if (ret < 2)
        return -1;
    return 1;
}
static
struct bt_trace_descriptor *scribe_open_trace(const char *path, int flags,
		void (*packet_seek)(struct bt_stream_pos *pos, size_t index,
			int whence), FILE *metadata_fp)
{
    struct scribe_stream_pos * scribe_pos;
    int ret;
    
	scribe_pos = g_new0(struct scribe_stream_pos, 1);
    ret = parse_url(path, scribe_pos);
    if (!ret) {
		fprintf(stderr, "[error] Error parsing scribe url.\n");
        return NULL;
    }
    printf("The hostname is %s and the port is %d\n", scribe_pos->hostname,
            scribe_pos->port);
	/*
     * Open connection with scribe server
     */
    scribe_pos->client = open_connection(scribe_pos->hostname, scribe_pos->port);

    scribe_pos->parent.rw_table = NULL;
	scribe_pos->parent.event_cb = scribe_write_event;
	scribe_pos->parent.trace = &scribe_pos->trace_descriptor;
	return &scribe_pos->trace_descriptor;

}

static
int scribe_close_trace(struct bt_trace_descriptor *td)
{
	struct scribe_stream_pos *pos =
		container_of(td, struct scribe_stream_pos,
			trace_descriptor);
    close_connection(pos->client);
	free(pos);
	return 0;
}

static
struct bt_format scribe_format = {
	.open_trace = scribe_open_trace,
	.close_trace = scribe_close_trace,
};

static
void __attribute__((constructor)) scribe_init(void)
{
	int ret;

	scribe_format.name = g_quark_from_static_string("scribe");
	ret = bt_register_format(&scribe_format);
	assert(!ret);
}

static
void __attribute__((destructor)) scribe_exit(void)
{
	bt_unregister_format(&scribe_format);
}
