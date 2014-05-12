/*
 * BabelTrace - Scribe Output
 */

#include <babeltrace/ctf-text/types.h>
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

static
struct bt_trace_descriptor *scribe_open_trace(const char *path, int flags,
		void (*packet_seek)(struct bt_stream_pos *pos, size_t index,
			int whence), FILE *metadata_fp)
{
	struct ctf_text_stream_pos *pos;

	pos = g_new0(struct ctf_text_stream_pos, 1);
	pos->parent.rw_table = NULL;
	pos->parent.event_cb = scribe_write_event;
	pos->parent.trace = &pos->trace_descriptor;
	return &pos->trace_descriptor;
}

static
int scribe_close_trace(struct bt_trace_descriptor *td)
{
	struct ctf_text_stream_pos *pos =
		container_of(td, struct ctf_text_stream_pos,
			trace_descriptor);
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
