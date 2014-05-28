/*
 * Common Trace Format
 */

#include <babeltrace/scribe/types.h>
#include <babeltrace/scribe/raw-scribe-types.h>
#include <stdio.h>

int scribe_raw_struct_write(struct bt_stream_pos *ppos, 
        struct bt_definition *definition)
{
	struct scribe_stream_pos *pos = container_of(ppos, struct scribe_stream_pos,
            parent);
	int field_nr_saved;
	int ret;
    int count;

	if (!print_field(definition))
		return 0;

	if (!pos->dummy) {
		if (pos->depth >= 0) {
			if (pos->field_nr++ != 0) {
                count = snprintf(pos->log_event, pos->log_count, ",");         
                pos->log_event += count;
                pos->log_count -= count; 
            }
            count = snprintf(pos->log_event, pos->log_count, " ");
            pos->log_event += count;
            pos->log_count -= count; 
			if (pos->print_names && definition->name != 0) {
                count = snprintf(pos->log_event, pos->log_count, "%s = ",
                    rem_(g_quark_to_string(definition->name)));
                pos->log_event += count;
                pos->log_count -= count; 
            }
            count = snprintf(pos->log_event, pos->log_count, "{");
            pos->log_event += count;
            pos->log_count -= count; 
		}
		pos->depth++;
	}
	field_nr_saved = pos->field_nr;
	pos->field_nr = 0;
	ret = bt_struct_rw(ppos, definition);
	if (!pos->dummy) {
		pos->depth--;
		if (pos->depth >= 0) {
            count = snprintf(pos->log_event, pos->log_count, "}");
            pos->log_event += count;
            pos->log_count -= count; 
		}
	}
	pos->field_nr = field_nr_saved;
	return ret;
}
