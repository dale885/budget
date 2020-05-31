/**
 * Copyright (C) 2020 Dallas Leclerc
 */

#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <unity.h>

#include <budget_db/budget_db.h>
#include <error.h>
#include <log.h>

#define TEST_NAME "budget_app_db_test"
#define DB_FILE "budget.db"
#define HOME_ENV "HOME"
#define SECONDS_IN_A_DAY 86400

db_connection db;

static void remove_db_file() {
	struct stat st;
	char* db_file_path = (char*)malloc(
		sizeof(char) * (strlen(db.db_path) + strlen(DB_FILE) + 2));

	TEST_ASSERT_NOT_NULL(db_file_path);

	strncpy(db_file_path, db.db_path, strlen(db.db_path));
	strncat(db_file_path, "/", 1);
	strncat(db_file_path, DB_FILE, strlen(DB_FILE));

	if (0 == stat(db_file_path, &st)) {
		DEBUG_LOG("Removing db file [%s]", db_file_path);
		remove(db_file_path);
	}

	free(db_file_path);
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

	NOTICE_LOG("Test [%s] completed with [%d] failures",
		TEST_NAME, num_failures);

	close_log();

	remove_db_file();

	if (db.db_path) {
		free(db.db_path);
	}

	return num_failures != 0 ? ERR_KO : ERR_OK;
}

void setUp() {
	TEST_ASSERT_EQUAL_INT(ERR_OK, open_budget_db(&db));
}

void tearDown() {
	if (db.handle) {
		close_budget_db(&db);
		db.handle = NULL;
	}
	remove_db_file();
}

void test_open_close_budget_db_with_invalid_arguments() {
	db_connection invalid = {0};

	TEST_ASSERT_EQUAL_INT(ERR_IN_USE, open_budget_db(&db));
	TEST_ASSERT_EQUAL_INT(ERR_OK, close_budget_db(&db));

	TEST_ASSERT_EQUAL_INT(ERR_INVALID, open_budget_db(NULL));
	TEST_ASSERT_EQUAL_INT(ERR_INVALID, open_budget_db(&invalid));

	TEST_ASSERT_EQUAL_INT(ERR_INVALID, close_budget_db(&invalid));
	TEST_ASSERT_EQUAL_INT(ERR_INVALID, close_budget_db(NULL));
}

void test_insert_expenses() {

	uint32_t i;
	expense_list expenses = {0};

	TEST_ASSERT_EQUAL_INT(ERR_INVALID, insert_expenses(NULL, &expenses));
	TEST_ASSERT_EQUAL_INT(ERR_INVALID, insert_expenses(&db, NULL));
	TEST_ASSERT_EQUAL_INT(ERR_INVALID, insert_expenses(&db, &expenses));

	expenses.num_expenses = 3;
	expenses.expenses = (expense*)malloc(
		sizeof(expense) * expenses.num_expenses);

	TEST_ASSERT_NOT_NULL(expenses.expenses);

	for (i = 0; i < expenses.num_expenses; ++i) {
		expenses.expenses[i].amount = i;
		expenses.expenses[i].payment_type = i;
		expenses.expenses[i].expense_type = i;
		expenses.expenses[i].date = time(NULL);
		expenses.expenses[i].description = "Test expense";
	}

	TEST_ASSERT_EQUAL_INT(ERR_OK, insert_expenses(&db, &expenses));

	free(expenses.expenses);
}

void test_get_expenses() {
	uint32_t i;
	expense_list expenses = {0};
	time_t expense_date = time(NULL);
	date_range range = {0};
	size_t num_expenses = 10;

	expenses.num_expenses = num_expenses;
	expenses.expenses = (expense*)malloc(
		sizeof(expense) * num_expenses);
	TEST_ASSERT_NOT_NULL(expenses.expenses);

	for (i = 0; i < num_expenses; ++i) {
		expenses.expenses[i].amount = i * 1.00;
		expenses.expenses[i].date = expense_date - i * SECONDS_IN_A_DAY;
		expenses.expenses[i].description = "Test expense";
		expenses.expenses[i].expense_type = i % 3;
		expenses.expenses[i].payment_type = i % 4;
	}

	TEST_ASSERT_EQUAL_INT(ERR_OK, insert_expenses(&db, &expenses));

	free(expenses.expenses);
	expenses.expenses = NULL;
	expenses.num_expenses = 0;

	range.end = time(NULL) - SECONDS_IN_A_DAY;
	range.start = time(NULL);
	TEST_ASSERT_EQUAL_INT(ERR_INVALID, get_expenses_in_range(&db, &range, &expenses));

	range.start = 0;
	range.end = 0;
	TEST_ASSERT_EQUAL_INT(ERR_OK, get_expenses_in_range(&db, &range, &expenses));
	TEST_ASSERT_NOT_NULL(expenses.expenses);
	TEST_ASSERT_EQUAL_UINT(num_expenses, expenses.num_expenses);

	free(expenses.expenses);
	expenses.expenses = NULL;
	expenses.num_expenses = 0;

	range.start = expense_date - (5 * SECONDS_IN_A_DAY);
	range.end = time(NULL);
	TEST_ASSERT_EQUAL_INT(ERR_OK, get_expenses_in_range(&db, &range, &expenses));
	TEST_ASSERT_NOT_NULL(expenses.expenses);
	TEST_ASSERT_EQUAL_UINT(5, expenses.num_expenses);

	free(expenses.expenses);
	expenses.expenses = NULL;
	expenses.num_expenses = 0;

	range.end = expense_date - (5 * SECONDS_IN_A_DAY - 1);
	range.start = 0;
	TEST_ASSERT_EQUAL_INT(ERR_OK, get_expenses_in_range(&db, &range, &expenses));

	TEST_ASSERT_NOT_NULL(expenses.expenses);
	TEST_ASSERT_EQUAL_UINT(5, expenses.num_expenses);

	free(expenses.expenses);
	expenses.expenses = NULL;
	expenses.num_expenses = 0;

	range.start = 0;
	range.end = 0;
	TEST_ASSERT_EQUAL_UINT(ERR_OK, get_expenses_in_range_with_payment_type(&db, &range, 0, &expenses));

	TEST_ASSERT_NOT_NULL(expenses.expenses);
	TEST_ASSERT_EQUAL_UINT(3, expenses.num_expenses);

	free(expenses.expenses);
	expenses.expenses = NULL;
	expenses.num_expenses = 0;

	TEST_ASSERT_EQUAL_UINT(ERR_OK, get_expenses_in_range_with_expense_type(&db, &range, 0, &expenses));

	TEST_ASSERT_NOT_NULL(expenses.expenses);
	TEST_ASSERT_EQUAL_UINT(4, expenses.num_expenses);

	free(expenses.expenses);
	expenses.expenses = NULL;
	expenses.num_expenses = 0;
}

int main() {
	UNITY_BEGIN();

	suiteSetUp();

	RUN_TEST(test_open_close_budget_db_with_invalid_arguments);
	RUN_TEST(test_insert_expenses);
	RUN_TEST(test_get_expenses);

	return suiteTearDown(UNITY_END());
}
