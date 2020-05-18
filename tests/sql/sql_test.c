/**
 * Copyright (C) 2020 Dallas Leclerc
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include <unity.h>

#include <sql/sql_db.h>
#include <error.h>
#include <log.h>

#define TEST_NAME "budget_app_sql_test"
#define DB_DIR "/tmp/"
#define DB_FILE_PATH "/tmp/db/budget.db"
sqlite3* db = NULL;

static void remove_db_file() {
	struct stat st;
	if (0 == stat(DB_FILE_PATH, &st)) {
		remove(DB_FILE_PATH);
	}
}

void suiteSetUp() {
	open_log(TEST_NAME);

	TEST_ASSERT_EQUAL_INT(ERR_OK, open_db(DB_DIR, &db));
	remove_db_file();
}

int32_t suiteTearDown(int32_t num_failures) {
	close_log();

	remove_db_file();

	return num_failures != 0 ? ERR_KO : ERR_OK;
}

void setUp() {
}


void tearDown() {
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
	struct db_query query;

	query.db = db;

	TEST_ASSERT_EQUAL_INT(ERR_INVALID, execute_query(NULL, NULL));
	TEST_ASSERT_EQUAL_INT(ERR_INVALID, execute_query(&query, NULL));

	query.query = NULL;
	TEST_ASSERT_EQUAL_INT(ERR_INVALID, execute_query(&query, NULL));

	query.num_params = 5;
	query.params = NULL;
	TEST_ASSERT_EQUAL_INT(ERR_INVALID, execute_query(&query, NULL));
}

void test_execute_with_invalid_results() {
}

void test_table_queries() {
}

void test_queries_with_invalid_params() {
}

void test_queries() {
}

int main() {
	UNITY_BEGIN();

	RUN_TEST(test_open_db_with_invalid_arguments);
	RUN_TEST(test_execute_with_invalid_query);
	RUN_TEST(test_execute_with_invalid_results);
	RUN_TEST(test_queries_with_invalid_params);
	RUN_TEST(test_table_queries);
	RUN_TEST(test_queries);

	return UNITY_END();
}

