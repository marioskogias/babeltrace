/*
 * Common Trace Format
 *
 * Floating point read/write functions.
 *
 */

#include <babeltrace/scribe/types.h>
#include <stdio.h>

int scribe_float_write(struct bt_stream_pos *ppos, struct bt_definition *definition)
{
	struct definition_float *float_definition =
		container_of(definition, struct definition_float, p);
	struct scribe_stream_pos *pos = container_of(ppos, struct scribe_stream_pos,
            parent);
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

    count = snprintf(pos->log_event, pos->log_count, "%g", 
            float_definition->value);
    pos->log_event += count;
    pos->log_count -= count; 
	return 0;
}
