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

static size_t get_num_rows(char* restrict csv_data) {

	size_t num_rows = 0;
	char* row;
	while ((row = strtok(csv_data, "\n"))) {
		++num_rows;
	}

	DEBUG_LOG("Got [%u] rows", num_rows);

	return num_rows;
}

static size_t get_num_columns(char* restrict csv_data, size_t csv_length) {
	size_t num_cols = 0;
	size_t i;
	bool done = false;
	bool in_text = false;

	for (i = 0; i < csv_length && !done; ++i) {
		switch (csv_data[i]) {
			case ',':
				if (!in_text) {
					++num_cols;
				}
				break;
			case '\n':
			case '\0':
				++num_cols;
				done = true;
				break;
			case '"':
				in_text = !in_text;
				break;
			default:
				break;
		}
	}

	return num_cols;
}

static int32_t parse_row(char* restrict csv_row, size_t csv_length, size_t num_cols ,budget_value* csv_value) {

	

}



static int32_t parse_csv_data(char* restrict raw_csv_data, size_t csv_length, bool has_header, csv_data* restrict data) {
	size_t num_rows;
	size_t i = has_header ? 1 : 0;

	num_rows = get_num_rows(raw_csv_data);
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
