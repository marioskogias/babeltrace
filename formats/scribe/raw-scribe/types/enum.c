/*
 * Common Trace Format
 *
 */

#include <babeltrace/scribe/types.h>
#include <babeltrace/scribe/raw-scribe-types.h>
#include <stdio.h>
#include <stdint.h>

int scribe_raw_enum_write(struct bt_stream_pos *ppos, 
        struct bt_definition *definition)
{
	struct definition_enum *enum_definition =
		container_of(definition, struct definition_enum, p);
	struct definition_integer *integer_definition =
		enum_definition->integer;
	struct scribe_stream_pos *pos = container_of(ppos, struct scribe_stream_pos,
            parent);
	GArray *qs;
	int ret;
   	int field_nr_saved;
    int count;

	if (!print_field(definition))
		return 0;

	if (pos->dummy)
		return 0;

    if (pos->field_nr++ != 0) {
        count = snprintf(pos->log_event, pos->log_count, ",");         
        pos->log_event += count;
        pos->log_count -= count; 
    }
    count = snprintf(pos->log_event, pos->log_count, " ");
    pos->log_event += count;
    pos->log_count -= count; 
	if (pos->print_names) {
        count = snprintf(pos->log_event, pos->log_count, "%s = ",
			rem_(g_quark_to_string(definition->name)));
        pos->log_event += count;
        pos->log_count -= count; 
    }

	field_nr_saved = pos->field_nr;
	pos->field_nr = 0;
    count = snprintf(pos->log_event, pos->log_count, "(");
    pos->log_event += count;
    pos->log_count -= count; 
	pos->depth++;
	qs = enum_definition->value;

	if (qs) {
		int i;

		for (i = 0; i < qs->len; i++) {
			GQuark q = g_array_index(qs, GQuark, i);
			const char *str = g_quark_to_string(q);

			assert(str);
			if (pos->field_nr++ != 0) {
                count = snprintf(pos->log_event, pos->log_count, ",");
                pos->log_event += count;
                pos->log_count -= count; 
            }
            count = snprintf(pos->log_event, pos->log_count, " \"%s\"", str);
            pos->log_event += count;
            pos->log_count -= count; 
		}
	} else {
        count = snprintf(pos->log_event, pos->log_count, "<unknown>");
        pos->log_event += count;
        pos->log_count -= count; 
	}

	pos->field_nr = 0;
    count = snprintf(pos->log_event, pos->log_count, ":");
    pos->log_event += count;
    pos->log_count -= count; 
	ret = generic_rw(ppos, &integer_definition->p);

	pos->depth--;
    count = snprintf(pos->log_event, pos->log_count, ")");
    pos->log_event += count;
    pos->log_count -= count; 
	pos->field_nr = field_nr_saved;
	return ret;
}
