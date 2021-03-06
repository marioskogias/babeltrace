/*
 * BabelTrace - Scribe Output
 */

#include <babeltrace/scribe/types.h>
#include <babeltrace/scribe/raw-scribe-types.h>
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

#define NSEC_PER_SEC 1000000000ULL

enum field_item {
	ITEM_SCOPE,
	ITEM_HEADER,
	ITEM_CONTEXT,
	ITEM_PAYLOAD,
};

static
struct bt_trace_descriptor *scribe_open_trace(const char *path, int flags,
		void (*packet_seek)(struct bt_stream_pos *pos, size_t index,
			int whence), FILE *metadata_fp);


static
struct bt_format scribe_raw_format = {
	.open_trace = scribe_open_trace,
	.close_trace = scribe_generic_close_trace,
};

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

static formatter raw_scribe_formatters[] = {
    raw_format_pre_payload,
    raw_format_payload
};

static
void __attribute__((constructor)) scribe_raw_init(void)
{
	int ret;

	scribe_raw_format.name = g_quark_from_static_string("scribe_raw");
	ret = bt_register_format(&scribe_raw_format);
	assert(!ret);
}

static
void __attribute__((destructor)) scribe_raw_exit(void)
{
	bt_unregister_format(&scribe_raw_format);
}

static
void set_field_names_print(struct scribe_stream_pos *pos, enum field_item item)
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

int print_field(struct bt_definition *definition)
{
	/* Always print all fields */
    return 1;
}


static
struct bt_trace_descriptor *scribe_open_trace(const char *path, int flags,
		void (*packet_seek)(struct bt_stream_pos *pos, size_t index,
			int whence), FILE *metadata_fp)
{
    struct bt_trace_descriptor *ret;
    ret = scribe_generic_open_trace(path, flags, NULL, metadata_fp);
    
    struct scribe_stream_pos *ppos = container_of(ret, struct scribe_stream_pos, 
            trace_descriptor);
    
    /* Set formatters table */
    ppos->formatters_table = raw_scribe_formatters;
    /* Set rw table */
    ppos->parent.rw_table = raw_scribe_rw;
    
    return ret;
}

int raw_format_pre_payload(struct bt_stream_pos *ppos, 
        struct ctf_stream_definition *stream)
{
    struct scribe_stream_pos *pos =
		container_of(ppos, struct scribe_stream_pos, parent);
	struct ctf_stream_declaration *stream_class = stream->stream_class;
	struct ctf_event_declaration *event_class;
	struct ctf_event_definition *event;
	uint64_t id;
	int dom_print = 0;
    int count;

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
        } else {
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
        } else {
			count = snprintf(pos->log_event, pos->log_count, " ");
            pos->log_event += count;
            pos->log_count -= count;
	    }
    }

	if (opt_delta_field && stream->has_timestamp) {
		uint64_t delta, delta_sec, delta_nsec;

		set_field_names_print(pos, ITEM_HEADER);
		if (pos->print_names) {
			count = snprintf(pos->log_event, pos->log_count, "delta =  ");
            pos->log_event += count;
            pos->log_count -= count;
	    } else {
			count = snprintf(pos->log_event, pos->log_count, "(");
            pos->log_event += count;
            pos->log_count -= count;
	    }
		if (pos->last_real_timestamp != -1ULL) {
			delta = stream->real_timestamp - pos->last_real_timestamp;
			delta_sec = delta / NSEC_PER_SEC;
			delta_nsec = delta % NSEC_PER_SEC;
			count = snprintf(pos->log_event, pos->log_count,
                    "+%" PRIu64 ".%09" PRIu64, delta_sec, delta_nsec);
            pos->log_event += count;
            pos->log_count -= count;
		} else {
			count = snprintf(pos->log_event, pos->log_count, "+?.?????????");
            pos->log_event += count;
            pos->log_count -= count;
		}
		if (!pos->print_names) {
			count = snprintf(pos->log_event, pos->log_count, ")");
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
		pos->last_real_timestamp = stream->real_timestamp;
		pos->last_cycles_timestamp = stream->cycles_timestamp;
	}
	if ((opt_trace_field || opt_all_fields) &&
            stream_class->trace->parent.path[0] != '\0') {
		set_field_names_print(pos, ITEM_HEADER);
		if (pos->print_names) {
			count = snprintf(pos->log_event, pos->log_count, "trace = ");
            pos->log_event += count;
            pos->log_count -= count;
		}
        count = snprintf(pos->log_event, pos->log_count, "%s",
                stream_class->trace->parent.path);
        pos->log_event += count;
        pos->log_count -= count;
        if (pos->print_names) {
            count = snprintf(pos->log_event, pos->log_count, ", ");
            pos->log_event += count;
            pos->log_count -= count;
        } else {
            count = snprintf(pos->log_event, pos->log_count, " ");
            pos->log_event += count;
            pos->log_count -= count;
        }
	}
	if ((opt_trace_hostname_field || opt_all_fields || opt_trace_default_fields)
			&& stream_class->trace->env.hostname[0] != '\0') {
		set_field_names_print(pos, ITEM_HEADER);
		if (pos->print_names) {
            count = snprintf(pos->log_event, pos->log_count,
                    "trace:hostname = ");
            pos->log_event += count;
            pos->log_count -= count;
		}
        count = snprintf(pos->log_event, pos->log_count, "%s",
                stream_class->trace->env.hostname);
        pos->log_event += count;
        pos->log_count -= count;
		if (pos->print_names) {
        count = snprintf(pos->log_event, pos->log_count, ", ");
        pos->log_event += count;
        pos->log_count -= count;
        }
		dom_print = 1;
	}

	if ((opt_trace_domain_field || opt_all_fields) &&
            stream_class->trace->env.domain[0] != '\0') {
		set_field_names_print(pos, ITEM_HEADER);
        if (pos->print_names) {
            count = snprintf(pos->log_event, pos->log_count,
                    "trace:domain = ");
            pos->log_event += count;
            pos->log_count -= count;
        }
        count = snprintf(pos->log_event, pos->log_count, "%s",
                stream_class->trace->env.domain);
        pos->log_event += count;
        pos->log_count -= count;
        if (pos->print_names) {
            count = snprintf(pos->log_event, pos->log_count, ", ");
            pos->log_event += count;
            pos->log_count -= count;
        }
		dom_print = 1;
	}
	if ((opt_trace_procname_field || opt_all_fields || opt_trace_default_fields)
			&& stream_class->trace->env.procname[0] != '\0') {
		set_field_names_print(pos, ITEM_HEADER);
		if (pos->print_names) {
            count = snprintf(pos->log_event, pos->log_count,
                    "trace:procname = ");
            pos->log_event += count;
            pos->log_count -= count;
		} else if (dom_print) {
            count = snprintf(pos->log_event, pos->log_count, ":");
            pos->log_event += count;
            pos->log_count -= count;
		}
        count = snprintf(pos->log_event, pos->log_count, "%s",
            stream_class->trace->env.procname);
        pos->log_event += count;
        pos->log_count -= count;
        if (pos->print_names) {
            count = snprintf(pos->log_event, pos->log_count, ", ");
            pos->log_event += count;
            pos->log_count -= count;
        }
        dom_print = 1;
	}
	if ((opt_trace_vpid_field || opt_all_fields || opt_trace_default_fields)
			&& stream_class->trace->env.vpid != -1) {
		set_field_names_print(pos, ITEM_HEADER);
		if (pos->print_names) {
            count = snprintf(pos->log_event, pos->log_count, "trace:vpid");
            pos->log_event += count;
            pos->log_count -= count;
		} else if (dom_print) {
            count = snprintf(pos->log_event, pos->log_count, ":");
            pos->log_event += count;
            pos->log_count -= count;
		}
        count = snprintf(pos->log_event, pos->log_count, "%d",
                stream_class->trace->env.vpid);
        pos->log_event += count;
        pos->log_count -= count;
        if (pos->print_names) {
            count = snprintf(pos->log_event, pos->log_count, ", ");
            pos->log_event += count;
            pos->log_count -= count;
        }
		dom_print = 1;
    }
    return 1;
}

int raw_format_payload(struct bt_stream_pos *ppos, 
        struct ctf_stream_definition *stream)
{
    struct scribe_stream_pos *pos =
		container_of(ppos, struct scribe_stream_pos, parent);
	int field_nr_saved;
	struct ctf_event_definition *event;
	int ret;
    int count;
    uint64_t id;
    
    id = stream->event_id;
    event = g_ptr_array_index(stream->events_by_id, id);    
    
    /* print cpuid field from packet context */
	if (stream->stream_packet_context) {
		if (pos->field_nr++ != 0) {
            count = snprintf(pos->log_event, pos->log_count, ", ");
            pos->log_event += count;
            pos->log_count -= count;
        }
		set_field_names_print(pos, ITEM_SCOPE);
		if (pos->print_names) {
            count = snprintf(pos->log_event, pos->log_count,
                    "stream.packet.context");
            pos->log_event += count;
            pos->log_count -= count;
        }
        field_nr_saved = pos->field_nr;
		pos->field_nr = 0;
		set_field_names_print(pos, ITEM_CONTEXT);
		ret = generic_rw(ppos, &stream->stream_packet_context->p);
		if (ret)
			return -1;
		pos->field_nr = field_nr_saved;
	}
    /* Read and print event payload */
	if (event->event_fields) {
		if (pos->field_nr++ != 0) {
            count = snprintf(pos->log_event, pos->log_count, ",");
            pos->log_event += count;
            pos->log_count -= count;
        }
		set_field_names_print(pos, ITEM_SCOPE);
		if (pos->print_names) {
            count = snprintf(pos->log_event, pos->log_count, "event.fields");
            pos->log_event += count;
            pos->log_count -= count;
        }
		field_nr_saved = pos->field_nr;
		pos->field_nr = 0;
		set_field_names_print(pos, ITEM_PAYLOAD);
		ret = generic_rw(ppos, &event->event_fields->p);
		if (ret)
			return -1;
		pos->field_nr = field_nr_saved;
	}

	return 1;
}
