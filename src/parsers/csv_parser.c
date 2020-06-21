/**
 * Copyright (C) 2020 Dallas Leclerc
 */


#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <parsers/csv_parser.h>
#include <log.h>

struct csv_parser {
	size_t data_start;
	size_t data_end;
	size_t num_cols;
	bool has_header;
	line_end line_ending;
	char* raw_csv_data;
	size_t csv_size;
};

void get_num_columns(csv_handle restrict handle) {
	bool in_quotes = false;
	size_t i;
	size_t num_columns = 0;

	for (i = 0; i < handle->data_end; ++i) {
		switch (handle->raw_csv_data[i]) {
			case ',':
				if (!in_quotes) {
					++num_columns;
				}
				break;
			case '"':
				if (in_quotes) {
					if (handle->data_end > (i + 1)) {
						if ('"' == handle->raw_csv_data[i + 1]) {
							++i;
						}
					}
				}
				else {
					in_quotes = true;
				}
				break;
			case '\r':
			case '\n':
				if (!in_quotes) {
					handle->num_cols = num_columns + 1;
					return;
				}
				break;
			default:
				break;
		}
	}

	handle->num_cols = num_columns + 1;
}

static char* get_escaped_field(const char* restrict unescaped_field, const size_t unescpaed_size, const size_t num_escapes) {
	size_t i, j;
	size_t escaped_field_size = unescpaed_size - num_escapes;
	char* escaped_field;

	DEBUG_LOG("Handing [%u] escapes in CSV field", num_escapes);
	escaped_field = (char*)malloc(sizeof(char) * escaped_field_size + 1);

	for (i = 0, j = 0; i < unescpaed_size && j < escaped_field_size; ++i, ++j) {
		if ('"' == unescaped_field[i]) {
			// This should not happen
			if (i >= unescpaed_size - 1) {
				ERR_LOG("Found escape character but it is the last character in the field");
				free(escaped_field);
				return NULL;
			}

			++i;
		}

		escaped_field[j] = unescaped_field[i];
	}

	escaped_field[escaped_field_size] = '\0';

	return escaped_field;
}

static int32_t parse_field(csv_handle restrict parse_data, char** restrict data) {
	bool in_quotes = false;
	bool done = false;
	size_t i = parse_data->data_start;
	size_t num_escapes = 0;
	size_t field_ending_size = 1;
	size_t field_size;

	for ( ; i < parse_data->data_end && !done; ++i) {
		switch (parse_data->raw_csv_data[parse_data->data_start]) {
			case '"':
				if (in_quotes) {
					if (i + 1 >= parse_data->data_end) {
						done = true;
						break;
					}
					if ('"' == parse_data->raw_csv_data[i + 1]) {
						++num_escapes;
						++i;
						break;
					}

					done = true;
				}
				else {
					in_quotes = true;
				}
				break;
			case ',':
				if (!in_quotes) {
					field_ending_size = 1;
					done = true;
				}
				break;
			case '\r':
				if (!in_quotes) {
					if (parse_data->data_end > (i + 1)) {
						if ('\n' == parse_data->raw_csv_data[i + 1]) {
							++i;
							field_ending_size = 2;
						}
					}
					else {
						field_ending_size = 1;
					}
					done = true;
					break;
				}
			case '\n':
				if (!in_quotes) {
					done = true;
					field_ending_size = 1;
				}
			default:
				break;
		}
	}

	if (!done) {
		ERR_LOG("Encountered end of data before field was parsed");
		return ERR_INVALID;
	}

	field_size = i - parse_data->data_start - field_ending_size;
	if (in_quotes) {
		field_size -= 2;
	}

	*data = (char*)malloc(sizeof(char) * (field_size + 1));
	if (!*data) {
		ERR_LOG("Failed to allocate memory for field");
		return ERR_NOMEM;
	}

	if (in_quotes) {
		memcpy(*data, parse_data->raw_csv_data + 1, field_size);
	}
	else {
		memcpy(*data, parse_data->raw_csv_data, field_size);
	}

	*data[field_size] = '\0';

	if (0 != num_escapes) {
		char* escaped_field = get_escaped_field(*data, field_size, num_escapes); 
		if (!escaped_field) {
			ERR_LOG("Failed to allocate memory for escaped field");
			return ERR_NOMEM;
		}

		free(*data);
		*data = escaped_field;
	}

	DEBUG_LOG("Got field [%s]",  *data);

	parse_data->data_start += i;

	return ERR_OK;
}


static int32_t parse_row(csv_handle restrict parse_data, size_t num_columns, char** column_data) {
	size_t i;
	int32_t rc;

	for (i = 0; i < num_columns; ++i) {
		rc = parse_field(parse_data, &column_data[i]);
		if (rc) {
			ERR_LOG("Failed to parse column");
			return ERR_KO;
		}
	}

	while (parse_data->data_start < parse_data->data_end &&
		   ('\n' == parse_data->raw_csv_data[parse_data->data_start] ||
		   '\r' == parse_data->raw_csv_data[parse_data->data_start])) {
		++parse_data->data_start;
	}

	return ERR_OK;
}

csv_handle csv_open(const char* restrict filename, bool has_header, const line_end* const restrict line_ending) {

	csv_handle handle = NULL;
	int32_t fd = -1;
	struct stat buf;

	DEBUG_LOG("Opening CSV file [%s] for parsing", filename);

	if ( 0 != stat(filename, &buf)) {
		ERR_LOG("Unable to stat file [%s]: %m", filename);
		goto CLEAN_UP;
	}

	fd = open(filename, O_RDONLY);
	if (0 > fd) {
		ERR_LOG("Failed to open file [%s]: %m", filename);
		goto CLEAN_UP;
	}

	handle = (csv_handle)malloc(sizeof(struct csv_parser));
	if (!handle) {
		ERR_LOG("Failed to allocate csv_handle");
		goto CLEAN_UP;
	}

	handle->raw_csv_data = mmap(NULL, buf.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	if (!handle->raw_csv_data) {
		ERR_LOG("Failed to map CSV file [%s] to memory", filename);
		goto CLEAN_UP;
	}

	handle->csv_size = buf.st_size;
	handle->has_header = has_header;
	if (!line_ending) {
		INFO_LOG("No line ending specified defaulting to DOS line ends");
		handle->line_ending = dos_line_end;
	}
	else {
		handle->line_ending = *line_ending;
	}
	handle->data_start = 0;
	handle->data_end = handle->csv_size / sizeof(char);

	get_num_columns(handle);

	INFO_LOG("Successfully prepared CSV file [%s] for parsing", filename);
	INFO_LOG("File size:[%u], has header:[%d], line_endings:[%d]", handle->csv_size, handle->has_header, handle->line_ending);

CLEAN_UP:
	if (0 >= fd) {
		close(fd);
	}

	if (handle &&
		!handle->raw_csv_data) {
		free(handle);
		handle = NULL;
	}

	return handle;
}

void csv_free_data(csv_data* restrict data) {
	size_t i;

	if (!data ||
		!data->data) {
		WARN_LOG("CSV data is null");
		return;
	}

	for (i = 0; i < data->num_cols; ++i) {
		if (data->data[i]) {
			free(data->data[i]);
		}
	}

	free(data->data);
}

