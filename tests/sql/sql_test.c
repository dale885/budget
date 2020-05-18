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
sqlite3* db = NULL;
char* DB_DIR;

static void remove_db_file() {

	char* db_file_path = (char*)malloc(
		sizeof(char) * (strlen(DB_DIR) + strlen(DB_FILE) + 2));

	TEST_ASSERT_NOT_NULL(db_file_path);

	strcpy(db_file_path, DB_DIR);
	strcat(db_file_path, "/");
	strcat(db_file_path, DB_FILE);

	struct stat st;
	if (0 == stat(db_file_path, &st)) {
		remove(db_file_path);
	}

	free(db_file_path);
}

void suiteSetUp() {
	open_log(TEST_NAME);

	const char* home_dir = getenv(HOME_ENV);
	TEST_ASSERT_NOT_NULL(home_dir);

	DB_DIR = (char*)malloc(sizeof(char) * (strlen(home_dir) + 1));
	TEST_ASSERT_NOT_NULL(DB_DIR);

	strcpy(DB_DIR, home_dir);

	NOTICE_LOG("Using directory [%s] for testing", DB_DIR);

	remove_db_file();
}

int32_t suiteTearDown(int32_t num_failures) {
	close_log();

	remove_db_file();

	if (DB_DIR) {
		free(DB_DIR);
	}

	return num_failures != 0 ? ERR_KO : ERR_OK;
}

void setUp() {
	TEST_ASSERT_EQUAL_INT(ERR_OK, open_db(DB_DIR, &db));
}


void tearDown() {
	close_db(db);
	db = NULL;
	remove_db_file();
}

void test_open_db_with_invalid_arguments() {
	TEST_ASSERT_EQUAL_INT(ERR_OK, close_db(db));

	TEST_ASSERT_EQUAL_INT(ERR_INVALID, open_db(NULL, &db));
	TEST_ASSERT_EQUAL_INT(ERR_INVALID, open_db(DB_DIR, NULL));
	TEST_ASSERT_EQUAL_INT(ERR_INVALID, open_db(NULL, NULL));

	TEST_ASSERT_EQUAL_INT(ERR_KO, open_db("/etc/", &db));
	TEST_ASSERT_EQUAL_INT(ERR_KO, open_db("//", &db));
	TEST_ASSERT_EQUAL_INT(ERR_KO, open_db("", &db));
}

void test_execute_with_invalid_query() {
	struct db_query query = { NULL, NULL, 0, NULL};

	query.db = db;

	TEST_ASSERT_EQUAL_INT(ERR_INVALID, execute_query(NULL, NULL));

	TEST_ASSERT_EQUAL_INT(ERR_INVALID, execute_query(&query, NULL));

	TEST_ASSERT_EQUAL_INT(ERR_INVALID, execute_query(&query, NULL));

	query.num_params = 5;
	TEST_ASSERT_EQUAL_INT(ERR_INVALID, execute_query(&query, NULL));
}

void test_execute_with_invalid_results() {
}

void test_table_queries() {
	struct db_query query = {NULL, NULL, 0, NULL};
	query.db = db;
	query.query = CREATE_TEST_TABLE;

	TEST_ASSERT_EQUAL_INT(ERR_OK, execute_query(&query, NULL));

	TEST_ASSERT_EQUAL_INT(ERR_KO, execute_query(&query, NULL));
}

void test_queries_with_invalid_params() {
}

void test_queries() {
}

int main() {
	UNITY_BEGIN();

	suiteSetUp();

	RUN_TEST(test_open_db_with_invalid_arguments);
	RUN_TEST(test_execute_with_invalid_query);
	RUN_TEST(test_execute_with_invalid_results);
	RUN_TEST(test_queries_with_invalid_params);
	RUN_TEST(test_table_queries);
	RUN_TEST(test_queries);

	return suiteTearDown(UNITY_END());
}

