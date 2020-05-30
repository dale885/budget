/**
 * Copyright (C) 2020 Dallas Leclerc
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <unity.h>

#include "sql_test_queries.h"
#include <sql/sql_db.h>
#include <error.h>
#include <log.h>

#define TEST_NAME "budget_app_sql_test"
#define DB_FILE "budget.db"
#define HOME_ENV "HOME"
db_connection db = {0};

static void free_params(query_param* params, uint32_t num_params) {
	for (uint32_t i = 0; i < num_params; ++i) {
		if (TEXT == params[i].param.type) {
			free(params[i].param.value.string_val);
		}
	}
}

static void free_results(db_query_result* results) {
	for (uint32_t i = 0; i < results->num_rows; ++i) {
		for (uint32_t j = 0; j < results->num_cols; ++j) {
			if (TEXT == results->values[i][j].type) {
				free(results->values[i][j].value.string_val);
			}
		}
		free(results->values[i]);
	}
}

static void remove_db_file() {

	char* db_file_path = (char*)malloc(
		sizeof(char) * (strlen(db.db_path) + strlen(DB_FILE) + 2));

	TEST_ASSERT_NOT_NULL(db_file_path);

	strcpy(db_file_path, db.db_path);
	strcat(db_file_path, "/");
	strcat(db_file_path, DB_FILE);

	struct stat st;
	if (0 == stat(db_file_path, &st)) {
		remove(db_file_path);
	}

	free(db_file_path);
}

static void create_test_table() {
	struct db_query query = {NULL, NULL, 0, NULL};
	query.handle = db.handle;
	query.query = CREATE_TEST_TABLE;

	TEST_ASSERT_EQUAL_INT(ERR_OK, execute_query(&query, NULL));
}

static void generate_insert_row_params(
	int32_t id,
	int32_t int_val,
	double double_val,
	const char* text_val,
	query_param** params) {

	*params = (query_param*)malloc(
		sizeof(query_param) * 4);

	(*params)[0].name = ID_PARAM;
	(*params)[0].param.type = INT;
	(*params)[0].param.value.int_val = id;

	(*params)[1].name = INT_VAL_PARAM;
	(*params)[1].param.type = INT;
	(*params)[1].param.value.int_val = int_val;

	(*params)[2].name = DOUBLE_VAL_PARAM;
	(*params)[2].param.type = DOUBLE;
	(*params)[2].param.value.double_val = double_val;

	(*params)[3].name = TEXT_VAL_PARAM;
	(*params)[3].param.type = TEXT;
	(*params)[3].param.value.string_val = (char*)malloc(strlen(text_val) + 1);
	TEST_ASSERT_NOT_NULL((*params)[3].param.value.string_val);
	strcpy((*params)[3].param.value.string_val, text_val);
}

static void insert_rows(
	int32_t* int_val,
	double* double_val,
	const char** text_val,
	uint32_t num_rows) {

	query_param** params = (query_param**)malloc(
		sizeof(query_param*));
	TEST_ASSERT_NOT_NULL(params);

	struct db_query query = {db.handle, INSERT_ROW, 4, NULL};
	for (uint32_t i = 0; i < num_rows; ++i) {
		generate_insert_row_params(
			i+1,
			int_val[i],
			double_val[i],
			text_val[i],
			params);

		query.params = *params;
		DEBUG_LOG("Inserting row [%u]", i);
		DEBUG_LOG("Got params [%d], [%d], [%f], [%s]",
			query.params[0].param.value.int_val,
			query.params[1].param.value.int_val,
			query.params[2].param.value.double_val,
			query.params[3].param.value.string_val);
		TEST_ASSERT_EQUAL_INT(ERR_OK, execute_query(&query, NULL));

		free_params(*params, 4);
		free(*params);
	}

	free(params);
}

void suiteSetUp() {
	open_log(TEST_NAME);

	const char* home_dir = getenv(HOME_ENV);
	TEST_ASSERT_NOT_NULL(home_dir);

	db.db_path = (char*)malloc(sizeof(char) * (strlen(home_dir) + 1));
	TEST_ASSERT_NOT_NULL(db.db_path);

	strcpy(db.db_path, home_dir);

	NOTICE_LOG("Using directory [%s] for testing", db.db_path);

	remove_db_file();
}

int32_t suiteTearDown(int32_t num_failures) {
	close_log();

	remove_db_file();

	if (db.db_path) {
		free(db.db_path);
	}

	return num_failures != 0 ? ERR_KO : ERR_OK;
}

void setUp() {
	TEST_ASSERT_EQUAL_INT(ERR_OK, open_db(&db));
}


void tearDown() {
	if (db.handle) {
		close_db(&db);
		db.handle = NULL;
	}
	remove_db_file();
}

void test_open_db_with_invalid_arguments() {
	db_connection test = {0};

	TEST_ASSERT_EQUAL_INT(ERR_IN_USE, open_db(&db));
	TEST_ASSERT_EQUAL_INT(ERR_OK, close_db(&db));

	TEST_ASSERT_EQUAL_INT(ERR_INVALID, open_db(&test));
	TEST_ASSERT_EQUAL_INT(ERR_INVALID, open_db(NULL));
}

void test_execute_with_invalid_query() {
	db_query query = {0};

	query.handle = db.handle;

	TEST_ASSERT_EQUAL_INT(ERR_INVALID, execute_query(NULL, NULL));

	TEST_ASSERT_EQUAL_INT(ERR_INVALID, execute_query(&query, NULL));

	TEST_ASSERT_EQUAL_INT(ERR_INVALID, execute_query(&query, NULL));

	query.num_params = 5;
	TEST_ASSERT_EQUAL_INT(ERR_INVALID, execute_query(&query, NULL));
}

void test_execute_with_results() {

	create_test_table();

	int32_t int_vals[5] = { 1, 2, 3, 4, 5 };
	double double_vals[5] = { 1.15, 2.3, 4.6, 8.2, 16.4 };
	const char* text_vals[5] = {
		"Row 1",
		"Row 2",
		"Row 3",
		"Row 4",
		"Row 5"
	};

	insert_rows(int_vals, double_vals, text_vals, 5);

	db_query query = {db.handle, SELECT_ROW_WITH_ID, 1, NULL};
	query.params = (query_param*)malloc(
		sizeof(query_param) * query.num_params);
	query.params->name = "$id_param";
	query.params->param.type = INT;
	query.params->param.value.int_val = 2;
	uint32_t result_index = 1;

	db_query_result result = {0};

	TEST_ASSERT_EQUAL_INT(ERR_OK, execute_query(&query, &result));


	TEST_ASSERT_EQUAL_UINT(4, result.num_cols);
	TEST_ASSERT_EQUAL_UINT(1, result.num_rows);
	TEST_ASSERT_TRUE(result.values[0][0].type == INT);
	TEST_ASSERT_EQUAL_INT(query.params->param.value.int_val, result.values[0][0].value.int_val);
	TEST_ASSERT_TRUE(result.values[0][1].type == INT);
	TEST_ASSERT_EQUAL_INT(int_vals[result_index], result.values[0][1].value.int_val);
	TEST_ASSERT_TRUE(result.values[0][2].type == DOUBLE);
	TEST_ASSERT_EQUAL_DOUBLE(double_vals[result_index], result.values[0][2].value.double_val);
	TEST_ASSERT_TRUE(result.values[0][3].type == TEXT);
	TEST_ASSERT_EQUAL_STRING(text_vals[result_index], result.values[0][3].value.string_val);

	free_results(&result);
	free(query.params);
}

void test_table_queries() {
	db_query query = {db.handle, CREATE_TEST_TABLE, 0, NULL};

	create_test_table();

	TEST_ASSERT_EQUAL_INT(ERR_KO, execute_query(&query, NULL));
}

void test_queries_with_invalid_params() {
	create_test_table();

	db_query query = {db.handle, INSERT_ROW, 0, NULL};

	TEST_ASSERT_EQUAL_INT(ERR_KO, execute_query(&query, NULL));

	query.num_params = 4;
	TEST_ASSERT_EQUAL_INT(ERR_KO, execute_query(&query, NULL));

	generate_insert_row_params(1, 9, 5.25, "test", &query.params);

	free(query.params[3].param.value.string_val);
	query.params[3].name = NULL;
	query.params[3].param.value.string_val = NULL;

	execute_query(&query, NULL);

	free(query.params);
}

int main() {
	UNITY_BEGIN();

	suiteSetUp();
	RUN_TEST(test_open_db_with_invalid_arguments);
	RUN_TEST(test_execute_with_invalid_query);
	RUN_TEST(test_execute_with_results);
	RUN_TEST(test_queries_with_invalid_params);
	RUN_TEST(test_table_queries);

	return suiteTearDown(UNITY_END());
}

