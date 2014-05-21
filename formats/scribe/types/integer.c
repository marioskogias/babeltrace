#include <babeltrace/scribe/types.h>
#include <stdio.h>
#include <inttypes.h>
#include <stdint.h>
#include <babeltrace/bitfield.h>

int scribe_integer_write(struct bt_stream_pos *ppos, struct bt_definition *definition)
{
	struct definition_integer *integer_definition =
		container_of(definition, struct definition_integer, p);
	const struct declaration_integer *integer_declaration =
		integer_definition->declaration;
	struct scribe_stream_pos *pos = container_of(pos, struct scribe_stream_pos, 
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
	if (pos->string
	    && (integer_declaration->encoding == CTF_STRING_ASCII
	      || integer_declaration->encoding == CTF_STRING_UTF8)) {

		if (!integer_declaration->signedness) {
			g_string_append_c(pos->string,
				(int) integer_definition->value._unsigned);
		} else {
			g_string_append_c(pos->string,
				(int) integer_definition->value._signed);
		}
		return 0;
	}

	switch (integer_declaration->base) {
	case 0:	/* default */
	case 10:
		if (!integer_declaration->signedness) {
            count = snprintf(pos->log_event, pos->log_count, "%" PRIu64,
				integer_definition->value._unsigned);
            pos->log_event += count;
            pos->log_count -= count; 
		} else {
			count = snprintf(pos->log_event, pos->log_count, "%" PRId64,
				integer_definition->value._signed);
            pos->log_event += count;
            pos->log_count -= count; 
		}
		break;
	case 2:
	{
		int bitnr;
		uint64_t v;

		if (!integer_declaration->signedness)
			v = integer_definition->value._unsigned;
		else
			v = (uint64_t) integer_definition->value._signed;

        count = snprintf(pos->log_event, pos->log_count, "0b");
        pos->log_event += count;
        pos->log_count -= count; 
		v = _bt_piecewise_lshift(v, 64 - integer_declaration->len);
		for (bitnr = 0; bitnr < integer_declaration->len; bitnr++) {
            count = snprintf(pos->log_event, pos->log_count, "%u", 
                    (v & (1ULL << 63)) ? 1 : 0);
            pos->log_event += count;
            pos->log_count -= count; 
			v = _bt_piecewise_lshift(v, 1);
		}
		break;
	}
	case 8:
	{
		uint64_t v;

		if (!integer_declaration->signedness)
			v = integer_definition->value._unsigned;
		else
			v = (uint64_t) integer_definition->value._signed;

        count = snprintf(pos->log_event, pos->log_count, "0%" PRIo64, v);
        pos->log_event += count;
        pos->log_count -= count; 
		break;
	}
	case 16:
	{
		uint64_t v;

		if (!integer_declaration->signedness)
			v = integer_definition->value._unsigned;
		else
			v = (uint64_t) integer_definition->value._signed;

        count = snprintf(pos->log_event, pos->log_count, "0x%" PRIX64, v);
        pos->log_event += count;
        pos->log_count -= count; 
		break;
	}
	default:
		return -EINVAL;
	}

	return 0;
}
