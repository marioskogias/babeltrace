/*
 * Common Trace Format
 *
 */

#include <babeltrace/scribe/types.h>
#include <babeltrace/scribe/raw-scribe-types.h>
#include <stdio.h>
#include <limits.h>		/* C99 limits */
#include <string.h>

int scribe_raw_string_write(struct bt_stream_pos *ppos,
			  struct bt_definition *definition)
{
	struct definition_string *string_definition =
		container_of(definition, struct definition_string, p);
	struct scribe_stream_pos *pos = container_of(ppos, struct scribe_stream_pos,
            parent);
    int count;

	assert(string_definition->value != NULL);

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

    count = snprintf(pos->log_event, pos->log_count,"\"%s\"",
        string_definition->value);
    pos->log_event += count;
    pos->log_count -= count; 
	return 0;
}
