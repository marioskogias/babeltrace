/*
 * sequence.c
 *
 * BabelTrace - Sequence Type Converter
 *
 * Copyright 2010, 2011 - Mathieu Desnoyers <mathieu.desnoyers@efficios.com>
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
 */

#include <babeltrace/compiler.h>
#include <babeltrace/format.h>
#include <inttypes.h>

#ifndef max
#define max(a, b)	((a) < (b) ? (b) : (a))
#endif

static
struct definition *_sequence_definition_new(struct declaration *declaration,
					struct definition_scope *parent_scope,
					GQuark field_name, int index);
static
void _sequence_definition_free(struct definition *definition);

int sequence_rw(struct stream_pos *pos, struct definition *definition)
{
	struct definition_sequence *sequence_definition =
		container_of(definition, struct definition_sequence, p);
	const struct declaration_sequence *sequence_declaration =
		sequence_definition->declaration;
	uint64_t len, oldlen, i;
	int ret;

	ret = generic_rw(pos, &sequence_definition->len->p);
	if (ret)
		return ret;
	len = sequence_definition->len->value._unsigned;
	/*
	 * Yes, large sequences could be _painfully slow_ to parse due
	 * to memory allocation for each event read. At least, never
	 * shrink the sequence. Note: the sequence GArray len should
	 * never be used as indicator of the current sequence length.
	 * One should always look at the sequence->len->value._unsigned
	 * value for that.
	 */
	oldlen = sequence_definition->elems->len;
	if (oldlen < len)
		g_ptr_array_set_size(sequence_definition->elems, len);

	for (i = oldlen; i < len; i++) {
		struct definition **field;
		GString *str;
		GQuark name;

		str = g_string_new("");
		g_string_printf(str, "[%" PRIu64 "]", i);
		(void) g_string_free(str, TRUE);
		name = g_quark_from_string(str->str);

		field = (struct definition **) &g_ptr_array_index(sequence_definition->elems, i);
		*field = sequence_declaration->elem->definition_new(sequence_declaration->elem,
					  sequence_definition->scope,
					  name, i);
		ret = generic_rw(pos, *field);
		if (ret)
			return ret;
	}
	return 0;
}

static
void _sequence_declaration_free(struct declaration *declaration)
{
	struct declaration_sequence *sequence_declaration =
		container_of(declaration, struct declaration_sequence, p);

	free_declaration_scope(sequence_declaration->scope);
	declaration_unref(&sequence_declaration->len_declaration->p);
	declaration_unref(sequence_declaration->elem);
	g_free(sequence_declaration);
}

struct declaration_sequence *
	sequence_declaration_new(struct declaration_integer *len_declaration,
			  struct declaration *elem_declaration,
			  struct declaration_scope *parent_scope)
{
	struct declaration_sequence *sequence_declaration;
	struct declaration *declaration;

	sequence_declaration = g_new(struct declaration_sequence, 1);
	declaration = &sequence_declaration->p;
	assert(!len_declaration->signedness);
	declaration_ref(&len_declaration->p);
	sequence_declaration->len_declaration = len_declaration;
	declaration_ref(elem_declaration);
	sequence_declaration->elem = elem_declaration;
	sequence_declaration->scope = new_declaration_scope(parent_scope);
	declaration->id = CTF_TYPE_SEQUENCE;
	declaration->alignment = max(len_declaration->p.alignment, elem_declaration->alignment);
	declaration->declaration_free = _sequence_declaration_free;
	declaration->definition_new = _sequence_definition_new;
	declaration->definition_free = _sequence_definition_free;
	declaration->ref = 1;
	return sequence_declaration;
}

static
struct definition *_sequence_definition_new(struct declaration *declaration,
				struct definition_scope *parent_scope,
				GQuark field_name, int index)
{
	struct declaration_sequence *sequence_declaration =
		container_of(declaration, struct declaration_sequence, p);
	struct definition_sequence *sequence;
	struct definition *len_parent;

	sequence = g_new(struct definition_sequence, 1);
	declaration_ref(&sequence_declaration->p);
	sequence->p.declaration = declaration;
	sequence->declaration = sequence_declaration;
	sequence->p.ref = 1;
	sequence->p.index = index;
	sequence->p.name = field_name;
	sequence->p.path = new_definition_path(parent_scope, field_name);
	sequence->scope = new_definition_scope(parent_scope, field_name);
	len_parent = sequence_declaration->len_declaration->p.definition_new(&sequence_declaration->len_declaration->p,
				sequence->scope,
				g_quark_from_static_string("length"), 0);
	sequence->len =
		container_of(len_parent, struct definition_integer, p);
	sequence->elems = g_ptr_array_new();
	return &sequence->p;
}

static
void _sequence_definition_free(struct definition *definition)
{
	struct definition_sequence *sequence =
		container_of(definition, struct definition_sequence, p);
	struct definition *len_definition = &sequence->len->p;
	uint64_t i;

	for (i = 0; i < sequence->elems->len; i++) {
		struct definition *field;

		field = g_ptr_array_index(sequence->elems, i);
		field->declaration->definition_free(field);
	}
	(void) g_ptr_array_free(sequence->elems, TRUE);
	len_definition->declaration->definition_free(len_definition);
	free_definition_scope(sequence->scope);
	declaration_unref(sequence->p.declaration);
	g_free(sequence);
}

uint64_t sequence_len(struct definition_sequence *sequence)
{
	return sequence->len->value._unsigned;
}

struct definition *sequence_index(struct definition_sequence *sequence, uint64_t i)
{
	if (i >= sequence->len->value._unsigned)
		return NULL;
	assert(i < sequence->elems->len);
	return g_ptr_array_index(sequence->elems, i);
}
