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

struct csv_parse_data {
	size_t data_start;
	size_t data_end;
	char* raw_csv_data;
} typedef csv_parse_data;

static size_t get_num_columns(const char* restrict csv_data, size_t csv_length) {
	bool is_escaped = false;
	bool in_quotes = false;
	size_t i;
	size_t num_columns = 0;

	for (i = 0; i < csv_length; ++i) {
		switch (csv_data[i]) {
			case ',':
				if (!in_quotes || is_escaped) {
					++num_columns;
					is_escaped = false;
				}
				break;
			case '"':
				if (in_quotes && is_escaped) {
					is_escaped = false;
				}
				else if (in_quotes) {
					is_escaped = true;
				}
				break;
			case '\n':
				if (!in_quotes || is_escaped) {
					++num_columns;
					return num_columns;
				}
				break;
			default:
				break;
		}
	}

	return num_columns + 1;
}

static size_t get_num_rows(const char* restrict csv_data, size_t csv_length) {
	bool is_escaped = false;
	bool in_quotes = false;
	bool parsing_row = false;
	size_t i;
	size_t num_rows = 0;

	for (i = 0; i < csv_length; ++i) {
		switch (csv_data[i]) {
			case '"':
				if (in_quotes && is_escaped) {
					is_escaped = false;
				}
				else if (in_quotes) {
					is_escaped = true;
				}
				parsing_row = true;
				break;
			case '\n':
				if (!in_quotes || is_escaped) {
					++num_rows;
					parsing_row = false;
				}
				break;
			default:
				parsing_row = true;
				break;
		}
	}

	return num_rows + (parsing_row ? 1 : 0);
}

static int32_t parse_field(csv_parse_data* restrict parse_data, char** restrict data) {
	bool in_quotes = false;
	bool done = false;
	char* escaped_field = NULL;
	size_t i = parse_data->data_start;
	size_t j;
	size_t k;
	size_t num_escapes = 0;
	size_t field_ending_size = 1;
	size_t field_size;
	size_t escaped_field_size;

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
			case '\n':
				if (!in_quotes) {
					done = true;
					if ('\r' == parse_data->raw_csv_data[i - 1]) {
						field_ending_size = 2;
					}
					else {
						field_ending_size = 1;
					}
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

	if (num_escapes) {
		DEBUG_LOG("Handing [%u] escapes in CSV field", num_escapes);
		escaped_field_size = field_size = num_escapes;
		escaped_field = (char*)malloc(sizeof(char) * escaped_field_size + 1);
		if (!escaped_field) {
			ERR_LOG("Failed to allocate memory for escaped field");
			return ERR_NOMEM;
		}

		for (j = 0, k = 0; j < field_size && k < escaped_field_size; ++j, ++k) {
			if ('"' == *data[j]) {
				// This should not happen
				if (j >= field_size - 1) {
					ERR_LOG("Found escape character but it is the last character in the field");
					return ERR_INVALID;
				}

				++j;
			}

			escaped_field[k] = *data[j];
		}

		escaped_field[escaped_field_size] = '\0';
		free(*data);
		*data = escaped_field;
	}

	DEBUG_LOG("Got field [%s]",  *data);

	parse_data->data_start += i;

	return ERR_OK;
}


static int32_t parse_row(csv_parse_data* restrict parse_data, size_t num_columns, char** column_data) {
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

static int32_t parse_csv_data(char* restrict raw_csv_data, size_t csv_length, bool has_header, csv_data* restrict data) {
	int32_t rc = ERR_OK;
	size_t i;
	size_t data_start = 0;
	csv_parse_data parse_data = {0};

	data->num_cols = get_num_columns(raw_csv_data, csv_length);
	data->num_rows = get_num_rows(raw_csv_data, csv_length);

	if (has_header) {
		data->num_rows -= 1;
	}

	data->data = (char***)calloc(sizeof(char**), data->num_rows);
	if (!data->data) {
		ERR_LOG("Failed to allocate csv data rows");
		rc = ERR_NOMEM;
		goto CLEAN_UP;
	}

	parse_data.data_end = csv_length;
	parse_data.raw_csv_data = raw_csv_data;

	for (i = 0; i < data->num_rows; ++i) {
		if (data_start >= csv_length) {
			ERR_LOG("Encountered end of CSV data before parsing all rows");
			rc = ERR_KO;
			goto CLEAN_UP;
		}

		if (has_header) {
			rc = parse_row(&parse_data, data->num_cols, NULL);
			if (rc) {
				ERR_LOG("Failed to parse header row");
				goto CLEAN_UP;
			}
		}

		data->data[i] = (char**)calloc(sizeof(char*), data->num_cols);
		if (!data->data[i]) {
			ERR_LOG("Failed to allocate column data");
			rc = ERR_NOMEM;
			goto CLEAN_UP;
		}

		rc = parse_row(&parse_data, data->num_cols, data->data[i]);
		if (rc) {
			ERR_LOG("Failed to parse row");
			goto CLEAN_UP;
		}
	}

CLEAN_UP:
	if (rc) {
		free_csv_data(data);
	}

	return rc;
}

int32_t parse_csv_file(const char* filename, bool has_header, csv_data* data) {
	int32_t rc;
	int32_t fd;
	void* map;
	struct stat buf;

	DEBUG_LOG("Parsing CSV file [%s]", filename);
	rc = stat(filename, &buf);
	if (0 != rc) {
		ERR_LOG("Failed to stat file [%s]: %m", filename);
		return rc;
	}

	fd = open(filename, O_RDONLY);
	if (0 > fd) {
		ERR_LOG("Failed to open file [%s]: %m", filename);
		return fd;
	}

	map = mmap(NULL, buf.st_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
	if (!map) {
		ERR_LOG("Failed to create memory map of CSV file");
		goto CLEAN_UP;
	}

	rc = parse_csv_data(map, buf.st_size / sizeof(char), has_header, data);
	if (ERR_OK != rc) {
		ERR_LOG("Failed to parse CSV data");
		goto CLEAN_UP;
	}

CLEAN_UP:

	if (0 >= fd) {
		close(fd);
	}

	if (map) {
		munmap(map, buf.st_size);
	}

	return ERR_OK;
}

void free_csv_data(csv_data* restrict data) {
	size_t i;
	size_t j;

	if (!data ||
		!data->data) {
		WARN_LOG("CSV data is null");
		return;
	}

	DEBUG_LOG("Freeing [%u] rows with [%u] columns of CSV data", data->num_rows, data->num_cols);

	for (i = 0; i < data->num_rows; ++i) {
		if (data->data[i]) {
			for (j = 0; j < data->num_cols; ++j) {
				free(data->data[i][j]);
			}
			free(data->data[i]);
		}
	}

	free(data->data);
}

