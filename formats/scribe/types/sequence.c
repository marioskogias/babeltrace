/*
 * Common Trace Format
 */

#include <babeltrace/scribe/types.h>
#include <stdio.h>

int scribe_sequence_write(struct bt_stream_pos *ppos, struct bt_definition *definition)
{
	struct scribe_stream_pos *pos = container_of(ppos, struct scribe_stream_pos,
            parent);
	struct definition_sequence *sequence_definition =
		container_of(definition, struct definition_sequence, p);
	struct declaration_sequence *sequence_declaration =
		sequence_definition->declaration;
	struct bt_declaration *elem = sequence_declaration->elem;
	int field_nr_saved;
	int ret = 0;
    int count;

	if (!print_field(definition))
		return 0;

	if (!pos->dummy) {
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
	}

	if (elem->id == CTF_TYPE_INTEGER) {
		struct declaration_integer *integer_declaration =
			container_of(elem, struct declaration_integer, p);

		if (integer_declaration->encoding == CTF_STRING_UTF8
		      || integer_declaration->encoding == CTF_STRING_ASCII) {

			if (!(integer_declaration->len == CHAR_BIT
			    && integer_declaration->p.alignment == CHAR_BIT)) {
				pos->string = sequence_definition->string;
				g_string_assign(sequence_definition->string, "");
				ret = bt_sequence_rw(ppos, definition);
				pos->string = NULL;
			}
            count = snprintf(pos->log_event, pos->log_count, "\"%s\"", 
                    sequence_definition->string->str);
            pos->log_event += count;
            pos->log_count -= count; 
			return ret;
		}
	}

	if (!pos->dummy) {
        count = snprintf(pos->log_event, pos->log_count, "["); 
        pos->log_event += count;
        pos->log_count -= count; 
		pos->depth++;
	}
	field_nr_saved = pos->field_nr;
	pos->field_nr = 0;
	ret = bt_sequence_rw(ppos, definition);
	if (!pos->dummy) {
		pos->depth--;
        count = snprintf(pos->log_event, pos->log_count, "]"); 
        pos->log_event += count;
        pos->log_count -= count; 
	}
	pos->field_nr = field_nr_saved;
	return ret;
}
