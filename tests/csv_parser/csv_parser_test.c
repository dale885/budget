/**
 * Copyright (C) 2020 Dallas Leclerc
 */


#include <unity.h>

#define TEST_DATA_DIRECTORY_ENV "BUDGET_TEST_DIR"
#define CSV_TEST_DATA_DIRECTORY "parsers/csv/"

void csv_parser_open_tests() {
}

void csv_parser_line_endings_test() {
}

void csv_parser_header_tests() {
}

void csv_parser_parsing_tests() {
}

void csv_parser_performance_test() {
}

int main() {
	UNITY_BEGIN();

	RUN_TEST(csv_parser_open_tests);
	RUN_TEST(csv_parser_line_endings_test);
	RUN_TEST(csv_parser_header_tests);
	RUN_TEST(csv_parser_parsing_tests);
	RUN_TEST(csv_parser_performance_test);

	return UNITY_END();
}
