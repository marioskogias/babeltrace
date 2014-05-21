/*
 * BabelTrace - Scribe Output
 */

#include <babeltrace/ctf-text/types.h>
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

#define BUF_SIZE  1024
#define CATEGORY "LTTng"

static char current_event[BUF_SIZE];

enum field_item {
	ITEM_SCOPE,
	ITEM_HEADER,
	ITEM_CONTEXT,
	ITEM_PAYLOAD,
};


static
void set_field_names_print(struct ctf_text_stream_pos *pos, enum field_item item)
{
	switch (item) {
	case ITEM_SCOPE:
		if (opt_all_field_names || opt_scope_field_names)
			pos->print_names = 1;
		else
			pos->print_names = 0;
		break;
	case ITEM_HEADER:
		if (opt_all_field_names || opt_header_field_names)
			pos->print_names = 1;
		else
			pos->print_names = 0;
		break;
	case ITEM_CONTEXT:
		if (opt_all_field_names || opt_context_field_names)
			pos->print_names = 1;
		else
			pos->print_names = 0;
		break;
	case ITEM_PAYLOAD:
		if (opt_all_field_names || opt_payload_field_names)
			pos->print_names = 1;
		else
			pos->print_names = 0;

		break;
	default:
		assert(0);
	}
}
static
int scribe_write_event(struct bt_stream_pos *ppos, struct ctf_stream_definition *stream)
{
	struct scribe_stream_pos *pos =
		container_of(ppos, struct scribe_stream_pos, parent);
	struct ctf_stream_declaration *stream_class = stream->stream_class;
	int field_nr_saved;
	struct ctf_event_declaration *event_class;
	struct ctf_event_definition *event;
	uint64_t id;
	int ret;
	int dom_print = 0;
    int count;

    pos->log_event = current_event; 
    pos->log_count = BUF_SIZE;
	
    id = stream->event_id;

	if (id >= stream_class->events_by_id->len) {
		fprintf(stderr, "[error] Event id %" PRIu64 " is outside range.\n", id);
		return -EINVAL;
	}
	event = g_ptr_array_index(stream->events_by_id, id);
	if (!event) {
		fprintf(stderr, "[error] Event id %" PRIu64 " is unknown.\n", id);
		return -EINVAL;
	}
	event_class = g_ptr_array_index(stream_class->events_by_id, id);
	if (!event_class) {
		fprintf(stderr, "[error] Event class id %" PRIu64 " is unknown.\n", id);
		return -EINVAL;
	}

	if (stream->has_timestamp) {
		set_field_names_print(pos, ITEM_HEADER);
		if (pos->print_names) {
			count = snprintf(pos->log_event, pos->log_count, "timestamp = ");
            pos->log_event += count;
            pos->log_count -= count;
        }
		else {
			count = snprintf(pos->log_event, pos->log_count, "[");
            pos->log_event += count;
            pos->log_count -= count;
        }
		if (opt_clock_cycles) {
            count = snprintf(pos->log_event, pos->log_count, "%020" PRIu64, 
                    stream->cycles_timestamp);
            pos->log_event += count;
            pos->log_count -= count;
		} else {
            count = snprintf(pos->log_event, pos->log_count, "%" PRId64, 
                    stream->real_timestamp);
            pos->log_event += count;
            pos->log_count -= count;
		}
		if (!pos->print_names) {
			count = snprintf(pos->log_event, pos->log_count, "]");
            pos->log_event += count;
            pos->log_count -= count;
        }
		if (pos->print_names) {
			count = snprintf(pos->log_event, pos->log_count, ", ");
            pos->log_event += count;
            pos->log_count -= count;
        }
		else {
			count = snprintf(pos->log_event, pos->log_count, " ");
            pos->log_event += count;
            pos->log_count -= count;
	    }
    }
    scribe_log(pos->client, CATEGORY, current_event);
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
