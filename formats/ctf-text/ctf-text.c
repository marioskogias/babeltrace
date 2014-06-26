/*
 * BabelTrace - Common Trace Format (CTF)
 *
 * CTF Text Format registration.
 *
 * Copyright 2010-2011 EfficiOS Inc. and Linux Foundation
 *
 * Author: Mathieu Desnoyers <mathieu.desnoyers@efficios.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <babeltrace/format.h>
#include <babeltrace/ctf-text/types.h>
#include <babeltrace/ctf/metadata.h>
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
#include <string.h>

#define NSEC_PER_SEC 1000000000ULL

int opt_all_field_names,
	opt_scope_field_names,
	opt_header_field_names,
	opt_context_field_names,
	opt_payload_field_names,
	opt_all_fields,
	opt_trace_field,
	opt_trace_domain_field,
	opt_trace_procname_field,
	opt_trace_vpid_field,
	opt_trace_hostname_field,
	opt_trace_default_fields = 1,
	opt_loglevel_field,
	opt_emf_field,
	opt_callsite_field,
	opt_delta_field = 1;

enum field_item {
	ITEM_SCOPE,
	ITEM_HEADER,
	ITEM_CONTEXT,
	ITEM_PAYLOAD,
};

enum bt_loglevel {
        BT_LOGLEVEL_EMERG                  = 0,
        BT_LOGLEVEL_ALERT                  = 1,
        BT_LOGLEVEL_CRIT                   = 2,
        BT_LOGLEVEL_ERR                    = 3,
        BT_LOGLEVEL_WARNING                = 4,
        BT_LOGLEVEL_NOTICE                 = 5,
        BT_LOGLEVEL_INFO                   = 6,
        BT_LOGLEVEL_DEBUG_SYSTEM           = 7,
        BT_LOGLEVEL_DEBUG_PROGRAM          = 8,
        BT_LOGLEVEL_DEBUG_PROCESS          = 9,
        BT_LOGLEVEL_DEBUG_MODULE           = 10,
        BT_LOGLEVEL_DEBUG_UNIT             = 11,
        BT_LOGLEVEL_DEBUG_FUNCTION         = 12,
        BT_LOGLEVEL_DEBUG_LINE             = 13,
        BT_LOGLEVEL_DEBUG                  = 14,
};

static
struct bt_trace_descriptor *ctf_text_open_trace(const char *path, int flags,
		void (*packet_seek)(struct bt_stream_pos *pos, size_t index,
			int whence), FILE *metadata_fp);
static
int ctf_text_close_trace(struct bt_trace_descriptor *descriptor);

static
rw_dispatch write_dispatch_table[] = {
	[ CTF_TYPE_INTEGER ] = ctf_text_integer_write,
	[ CTF_TYPE_FLOAT ] = ctf_text_float_write,
	[ CTF_TYPE_ENUM ] = ctf_text_enum_write,
	[ CTF_TYPE_STRING ] = ctf_text_string_write,
	[ CTF_TYPE_STRUCT ] = ctf_text_struct_write,
	[ CTF_TYPE_VARIANT ] = ctf_text_variant_write,
	[ CTF_TYPE_ARRAY ] = ctf_text_array_write,
	[ CTF_TYPE_SEQUENCE ] = ctf_text_sequence_write,
};

static
struct bt_format ctf_text_format = {
	.open_trace = ctf_text_open_trace,
	.close_trace = ctf_text_close_trace,
};

static GQuark Q_STREAM_PACKET_CONTEXT_TIMESTAMP_BEGIN,
	Q_STREAM_PACKET_CONTEXT_TIMESTAMP_END,
	Q_STREAM_PACKET_CONTEXT_EVENTS_DISCARDED,
	Q_STREAM_PACKET_CONTEXT_CONTENT_SIZE,
	Q_STREAM_PACKET_CONTEXT_PACKET_SIZE;

/*this is the struct finally written to the file descriptor*/
struct zipkin_trace {
    char trace_name[20];
    char service_name[20];
    int port;
    char ip;
    long trace_id;
    long span_id;
    long parent_span_id;
    int kind; // 0 for timestamp 1 for key-val
    char key[20];
    char val[50];
    uint64_t timestamp;
};

static
void __attribute__((constructor)) init_quarks(void)
{
	Q_STREAM_PACKET_CONTEXT_TIMESTAMP_BEGIN = g_quark_from_static_string("stream.packet.context.timestamp_begin");
	Q_STREAM_PACKET_CONTEXT_TIMESTAMP_END = g_quark_from_static_string("stream.packet.context.timestamp_end");
	Q_STREAM_PACKET_CONTEXT_EVENTS_DISCARDED = g_quark_from_static_string("stream.packet.context.events_discarded");
	Q_STREAM_PACKET_CONTEXT_CONTENT_SIZE = g_quark_from_static_string("stream.packet.context.content_size");
	Q_STREAM_PACKET_CONTEXT_PACKET_SIZE = g_quark_from_static_string("stream.packet.context.packet_size");
}

static
struct ctf_callsite_dups *ctf_trace_callsite_lookup(struct ctf_trace *trace,
			GQuark callsite_name)
{
	return g_hash_table_lookup(trace->callsites,
			(gpointer) (unsigned long) callsite_name);
}

int print_field(struct bt_definition *definition)
{
	/* Print all fields in verbose mode */
	if (babeltrace_verbose)
		return 1;

	/* Filter out part of the packet context */
	if (definition->path == Q_STREAM_PACKET_CONTEXT_TIMESTAMP_BEGIN)
		return 0;
	if (definition->path == Q_STREAM_PACKET_CONTEXT_TIMESTAMP_END)
		return 0;
	if (definition->path == Q_STREAM_PACKET_CONTEXT_EVENTS_DISCARDED)
		return 0;
	if (definition->path == Q_STREAM_PACKET_CONTEXT_CONTENT_SIZE)
		return 0;
	if (definition->path == Q_STREAM_PACKET_CONTEXT_PACKET_SIZE)
		return 0;

	return 1;
}

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
const char *print_loglevel(int value)
{
	switch (value) {
	case -1:
		return "";
	case BT_LOGLEVEL_EMERG:
		return "TRACE_EMERG";
	case BT_LOGLEVEL_ALERT:
		return "TRACE_ALERT";
	case BT_LOGLEVEL_CRIT:
		return "TRACE_CRIT";
	case BT_LOGLEVEL_ERR:
		return "TRACE_ERR";
	case BT_LOGLEVEL_WARNING:
		return "TRACE_WARNING";
	case BT_LOGLEVEL_NOTICE:
		return "TRACE_NOTICE";
	case BT_LOGLEVEL_INFO:
		return "TRACE_INFO";
	case BT_LOGLEVEL_DEBUG_SYSTEM:
		return "TRACE_DEBUG_SYSTEM";
	case BT_LOGLEVEL_DEBUG_PROGRAM:
		return "TRACE_DEBUG_PROGRAM";
	case BT_LOGLEVEL_DEBUG_PROCESS:
		return "TRACE_DEBUG_PROCESS";
	case BT_LOGLEVEL_DEBUG_MODULE:
		return "TRACE_DEBUG_MODULE";
	case BT_LOGLEVEL_DEBUG_UNIT:
		return "TRACE_DEBUG_UNIT";
	case BT_LOGLEVEL_DEBUG_FUNCTION:
		return "TRACE_DEBUG_FUNCTION";
	case BT_LOGLEVEL_DEBUG_LINE:
		return "TRACE_DEBUG_LINE";
	case BT_LOGLEVEL_DEBUG:
		return "TRACE_DEBUG";
	default:
		return "<<UNKNOWN>>";
	}
}

static
int ctf_text_write_event(struct bt_stream_pos *ppos, struct ctf_stream_definition *stream)
			 
{
	struct ctf_text_stream_pos *pos =
		container_of(ppos, struct ctf_text_stream_pos, parent);
	struct ctf_stream_declaration *stream_class = stream->stream_class;
	int field_nr_saved;
	struct ctf_event_declaration *event_class;
	struct ctf_event_definition *event;
	uint64_t id;
	int ret;
	int dom_print = 0;
    struct zipkin_trace ztrace;
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
    
    /*timestamp*/
    ztrace.timestamp = stream->real_timestamp;
    
    /*now the trace fields*/
    struct definition_struct *struct_definition =
		container_of(&event->event_fields->p, struct definition_struct, p);
    
    /*trace name*/
    struct bt_definition *field =
        g_ptr_array_index(struct_definition->fields, 0);
	
    struct definition_string *string_definition =
		container_of(field, struct definition_string, p);
	
    memcpy(&ztrace.trace_name, string_definition->value, 20);

    /*service name*/
    field = g_ptr_array_index(struct_definition->fields, 1);
    string_definition = container_of(field, struct definition_string, p);
    memcpy(&ztrace.service_name, string_definition->value, 20);

    /*port no*/
    field = g_ptr_array_index(struct_definition->fields, 2);
    struct definition_integer * integer_definition =	
        container_of(field, struct definition_integer, p);
    ztrace.port = integer_definition->value._unsigned;
    
    /*ip*/
    field = g_ptr_array_index(struct_definition->fields, 3);
    string_definition = container_of(field, struct definition_string, p);
    memcpy(&ztrace.ip, string_definition->value, 20);
	 
    /*trace id*/ 
    field = g_ptr_array_index(struct_definition->fields, 4);
    integer_definition = container_of(field, struct definition_integer, p);
    ztrace.trace_id = integer_definition->value._unsigned;
    
    /*span id*/ 
    field = g_ptr_array_index(struct_definition->fields, 5);
    integer_definition = container_of(field, struct definition_integer, p);
    ztrace.span_id = integer_definition->value._unsigned;
    
    /*parent span id*/ 
    field = g_ptr_array_index(struct_definition->fields, 6);
    integer_definition = container_of(field, struct definition_integer, p);
    ztrace.parent_span_id = integer_definition->value._unsigned;
    
    /*keyval or timestamp*/
    if (strcmp(g_quark_to_string(event_class->name), "zipkin:timestamp")) {
        //fprintf(stderr, "The event is keyval\n");
        field = g_ptr_array_index(struct_definition->fields, 7);
        string_definition = container_of(field, struct definition_string, p);
        memcpy(&ztrace.key, string_definition->value, 20);
        
        field = g_ptr_array_index(struct_definition->fields, 8);
        string_definition = container_of(field, struct definition_string, p);
        memcpy(&ztrace.val, string_definition->value, 20);
    
        ztrace.kind = 1; 
    } else {
        //fprintf(stderr, "The event is timestamp\n");
        field = g_ptr_array_index(struct_definition->fields, 7);
        string_definition = container_of(field, struct definition_string, p);
        memcpy(&ztrace.val, string_definition->value, 20);
    
        ztrace.kind = 0; 
    }
    
    /* newline */
	//fprintf(pos->fp, "\n");
    
    fwrite(&ztrace, sizeof(struct zipkin_trace), 1, pos->fp);
    fflush(pos->fp);
	pos->field_nr = 0;

	return 0;

error:
	fprintf(stderr, "[error] Unexpected end of stream. Either the trace data stream is corrupted or metadata description does not match data layout.\n");
	return ret;
}

static
struct bt_trace_descriptor *ctf_text_open_trace(const char *path, int flags,
		void (*packet_seek)(struct bt_stream_pos *pos, size_t index,
			int whence), FILE *metadata_fp)
{
	struct ctf_text_stream_pos *pos;
	FILE *fp;

	pos = g_new0(struct ctf_text_stream_pos, 1);

	pos->last_real_timestamp = -1ULL;
	pos->last_cycles_timestamp = -1ULL;
	switch (flags & O_ACCMODE) {
	case O_RDWR:
		if (!path)
			fp = stdout;
		else
			fp = fopen(path, "w");
		if (!fp)
			goto error;
		pos->fp = fp;
		pos->parent.rw_table = write_dispatch_table;
		pos->parent.event_cb = ctf_text_write_event;
		pos->parent.trace = &pos->trace_descriptor;
		pos->print_names = 0;
		babeltrace_ctf_console_output++;
		break;
	case O_RDONLY:
	default:
		fprintf(stderr, "[error] Incorrect open flags.\n");
		goto error;
	}

	return &pos->trace_descriptor;
error:
	g_free(pos);
	return NULL;
}

static
int ctf_text_close_trace(struct bt_trace_descriptor *td)
{
	int ret;
	struct ctf_text_stream_pos *pos =
		container_of(td, struct ctf_text_stream_pos, trace_descriptor);

	babeltrace_ctf_console_output--;
	if (pos->fp != stdout) {
		ret = fclose(pos->fp);
		if (ret) {
			perror("Error on fclose");
			return -1;
		}
	}
	g_free(pos);
	return 0;
}

static
void __attribute__((constructor)) ctf_text_init(void)
{
	int ret;

	ctf_text_format.name = g_quark_from_static_string("text");
	ret = bt_register_format(&ctf_text_format);
	assert(!ret);
}

static
void __attribute__((destructor)) ctf_text_exit(void)
{
	bt_unregister_format(&ctf_text_format);
}
