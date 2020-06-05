/**
 * Copyright (C) 2020 Dallas Leclerc
 */

#include <stdlib.h>

#include <budget_types.h>

void free_value(budget_value* restrict value) {
	if (!value) {
		return;
	}

	if (STRING == value->type && value->value.string_value) {
		free(value->value.string_value);
		value->value.string_value = NULL;
	}
}

void free_values(budget_value* restrict values, size_t num_values) {
	size_t i;

	if (!values) {
		return;
	}

	for (i = 0; i < num_values; ++i) {
		if (STRING == values[i].type && values[i].value.string_value) {
			free(values[i].value.string_value);
			values[i].value.string_value = NULL;
		}
	}
}

