/**
 * Copyright (C) 2020 Dallas Leclerc
 */

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sqlite3.h>

#include <budget_db/budget_db.h>
#include <budget_db/budget_db_queries.h>
#include <sql/sql_db.h>
#include <error.h>
#include <log.h>

static void result_to_expense(db_query_result* restrict result, expense_list* restrict expenses) {

	size_t i;
	size_t description_length;
	expenses->num_expenses = result->num_rows;
	expenses->expenses = (expense*)malloc(sizeof(expense) * result->num_rows);
	for (i = 0; i < expenses->num_expenses; ++i) {
		DEBUG_LOG("Got result row [%f, %ld, %u, %u, %s]",
			result->values[i][AMOUNT_INDEX].value.double_value,
			result->values[i][DATE_INDEX].value.int_value,
			result->values[i][PAYMENT_TYPE_INDEX].value.int_value,
			result->values[i][EXPENSE_TYPE_INDEX].value.int_value,
			result->values[i][DESCRIPTION_INDEX].value.string_value);

		expenses->expenses[i].amount = result->values[i][AMOUNT_INDEX].value.double_value;

		expenses->expenses[i].date = result->values[i][DATE_INDEX].value.int_value;
		expenses->expenses[i].payment_type = result->values[i][PAYMENT_TYPE_INDEX].value.int_value;
		expenses->expenses[i].expense_type = result->values[i][EXPENSE_TYPE_INDEX].value.int_value;

		description_length = strlen(result->values[i][DESCRIPTION_INDEX].value.string_value) + 1;

		expenses->expenses[i].description = (char*)malloc(
			sizeof(char) * description_length);
		strncpy(
			expenses->expenses[i].description,
			result->values[i][DESCRIPTION_INDEX].value.string_value,
			description_length);
	}
}

int32_t open_budget_db(db_connection* db) {
	int32_t rc;
	db_query query = {0};

	if (!db) {
		ERR_LOG("DB db is NULL");
		return ERR_INVALID;
	}

	if (db->handle) {
		WARN_LOG("Connection to budget DB is already open");
		return ERR_IN_USE;
	}

	DEBUG_LOG("Opening db to budget DB");

	rc = open_db(db);
	if (ERR_OK != rc) {
		ERR_LOG("Failed to open db: [%d:%s]",
			rc, error_to_string(rc));
		return rc;
	}

	query.handle = db->handle;
	query.query = CREATE_EXPENSES_TABLE;
	rc = execute_query(&query, NULL);
	if (ERR_OK != rc) {
		ERR_LOG("Failed to create expenses table");
		return rc;
	}


	return ERR_OK;
}

int32_t close_budget_db(db_connection* db) {
	int32_t rc;

	if (!db) {
		ERR_LOG("DB db is NULL");
		return ERR_INVALID;
	}

	DEBUG_LOG("Closing db to budget DB");

	if (db->handle) {
		rc = close_db(db);
		if (ERR_OK != rc) {
			ERR_LOG("Failed to close db to budget DB: [%d:%s]",
				rc, error_to_string(rc));
			return rc;
		}
	}
	else {
		WARN_LOG("DB handle is NULL");
		return ERR_INVALID;
	}

	return ERR_OK;
}

int32_t insert_expenses(db_connection* db, expense_list* expenses) {
	db_query query = {0};
	db_query_result result = {0};
	int32_t rc;
	size_t i;
	uint32_t next_id;
	size_t description_length;
	bool transaction_started = false;

	if (!db) {
		ERR_LOG("DB connection is NULL");
		return ERR_INVALID;
	}

	if (!db->handle) {
		ERR_LOG("No conection to DB available");
		return ERR_NOT_READY;
	}

	if (!expenses) {
		ERR_LOG("Expenses structure is NULL");
		return ERR_INVALID;
	}

	if (!expenses->num_expenses ||
		!expenses->expenses) {
		ERR_LOG("No expenses to add");
		return ERR_INVALID;
	}

	query.handle = db->handle;
	query.query = SELECT_NUM_ROWS;
	rc = execute_query(&query, &result);
	if (ERR_OK != rc) {
		ERR_LOG("Failed to get current number of rows");
		goto CLEAN_UP;
	}

	if (1 != result.num_rows) {
		ERR_LOG("Result is empty");
		rc = ERR_INVALID;
		goto CLEAN_UP;
	}

	if (INT != result.values[0][0].type) {
		ERR_LOG("Received a non integer for number of rows query");
		rc = ERR_INVALID;
		goto CLEAN_UP;
	}
	next_id = result.values[0][0].value.int_value + 1;

	query.query = BEGIN_TRANSACTION;
	rc = execute_query(&query, NULL);
	if (ERR_OK != rc) {
		ERR_LOG("Failed to begin SQL transaction");
		goto CLEAN_UP;
	}

	query.query = INSERT_EXPENSE;
	query.num_params = NUM_EXPENSE_PARAMS;
	query.params = (query_param*)malloc(sizeof(query_param) * NUM_EXPENSE_PARAMS);
	if (!query.params) {
		ERR_LOG("Failed to allocate query params");
		rc = ERR_NOMEM;
		goto CLEAN_UP;
	}

	for (i = 0; i < expenses->num_expenses; ++i) {
		query.params[ID_INDEX].name = ID_PARAM;
		query.params[ID_INDEX].param.type = INT;
		query.params[ID_INDEX].param.value.int_value = next_id + i;

		query.params[AMOUNT_INDEX].name = AMOUNT_PARAM;
		query.params[AMOUNT_INDEX].param.type = DOUBLE;
		query.params[AMOUNT_INDEX].param.value.double_value = expenses->expenses[i].amount; 

		query.params[DATE_INDEX].name = DATE_PARAM;
		query.params[DATE_INDEX].param.type = INT;
		query.params[DATE_INDEX].param.value.int_value = expenses->expenses[i].date; 

		query.params[PAYMENT_TYPE_INDEX].name = PAYMENT_TYPE_PARAM;
		query.params[PAYMENT_TYPE_INDEX].param.type = INT;
		query.params[PAYMENT_TYPE_INDEX].param.value.int_value = expenses->expenses[i].payment_type; 

		query.params[EXPENSE_TYPE_INDEX].name = EXPENSE_TYPE_PARAM;
		query.params[EXPENSE_TYPE_INDEX].param.type = INT;
		query.params[EXPENSE_TYPE_INDEX].param.value.int_value = expenses->expenses[i].expense_type; 

		description_length = strlen(expenses->expenses[i].description) + 1;
		query.params[DESCRIPTION_INDEX].name = DESCRIPTION_PARAM;
		query.params[DESCRIPTION_INDEX].param.type = STRING;
		query.params[DESCRIPTION_INDEX].param.value.string_value = (char*)malloc(sizeof(char) * description_length);
		strncpy(query.params[DESCRIPTION_INDEX].param.value.string_value, expenses->expenses[i].description, description_length);

		rc = execute_query(&query, NULL);
		if (ERR_OK != rc) {
			ERR_LOG("Failed to insert expense");
			goto CLEAN_UP;
		}

		free_params(query.params, query.num_params);
	}

CLEAN_UP:

	if (query.params) {
		free_params(query.params, query.num_params);
		free(query.params);
	}

	free_results(&result);

	if (transaction_started) {
		query.query = END_TRANSACTION;
		query.params = NULL;
		query.num_params = 0;
		if (ERR_OK != execute_query(&query, NULL)) {
			WARN_LOG("Failed to end transaction");
			rc = (rc == ERR_OK) ? ERR_KO : rc;
		}
	}

	return rc;
}

int32_t get_expenses_in_range(
	db_connection* db,
	date_range* range,
	expense_list* expenses) {

	db_query query = {0};
	db_query_result result = {0};
	int32_t rc;

	if (!db) {
		ERR_LOG("DB connection is NULL");
		return ERR_INVALID;
	}

	if (!db->handle) {
		ERR_LOG("No conection to DB available");
		return ERR_NOT_READY;
	}

	if (!expenses) {
		ERR_LOG("Expenses structure is NULL");
		return ERR_INVALID;
	}

	if (!range) {
		ERR_LOG("Date range is NULL");
		return ERR_INVALID;
	}

	query.handle = db->handle;
	query.query = SELECT_EXPENSES_IN_RANGE;
	query.num_params = NUM_RANGE_PARAMS;
	query.params = (query_param*)malloc(sizeof(query_param) * NUM_RANGE_PARAMS);

	if (!query.params) {
		ERR_LOG("Failed to allocate memory for params");
		return ERR_NOMEM;
	}

	query.params[START_DATE_INDEX].name = START_DATE_PARAM;
	query.params[START_DATE_INDEX].param.type = INT;
	query.params[START_DATE_INDEX].param.value.int_value = range->start;

	query.params[END_DATE_INDEX].name = END_DATE_PARAM;
	query.params[END_DATE_INDEX].param.type = INT;
	query.params[END_DATE_INDEX].param.value.int_value = range->end;

	DEBUG_LOG("Getting expenses in date range [%d:%d]", range->start, range->end);

	rc = execute_query(&query, &result);
	if (ERR_OK != rc) {
		ERR_LOG("Failed to get expenses");
		goto CLEAN_UP;
	}

	if (!result.num_rows) {
		INFO_LOG("No expenses matching query found");
		return ERR_OK;
	}

	if (NUM_EXPENSE_PARAMS != result.num_cols) {
		ERR_LOG("Received [%u] result columns but was expecting [%u]", result.num_cols, NUM_EXPENSE_PARAMS);
		goto CLEAN_UP;
	}

	result_to_expense(&result, expenses);

CLEAN_UP:

	if (query.params) {
		free(query.params);
	}

	free_results(&result);

	return rc;
}

int32_t get_expenses_in_range_with_payment_type(
	db_connection* db,
	date_range* range,
	uint32_t payment_type,
	expense_list* expenses) {

	int32_t rc;
	db_query query = {0};
	db_query_result result = {0};

	if (!db) {
		ERR_LOG("DB connection is NULL");
		return ERR_INVALID;
	}

	if (!db->handle) {
		ERR_LOG("No conection to DB available");
		return ERR_NOT_READY;
	}

	if (!expenses) {
		ERR_LOG("Expenses structure is NULL");
		return ERR_INVALID;
	}

	if (!range) {
		ERR_LOG("Date range is NULL");
		return ERR_INVALID;
	}

	query.handle = db->handle;
	query.query = SELECT_EXPENSES_IN_RANGE_WITH_PAYMENT_TYPE;
	query.num_params = NUM_RANGE_PARAMS_WITH_TYPE;
	query.params = (query_param*)malloc(sizeof(query_param) * NUM_RANGE_PARAMS_WITH_TYPE);

	if (!query.params) {
		ERR_LOG("Failed to allocate memory for params");
		return ERR_NOMEM;
	}

	query.params[START_DATE_INDEX].name = START_DATE_PARAM;
	query.params[START_DATE_INDEX].param.type = INT;
	query.params[START_DATE_INDEX].param.value.int_value = range->start;

	query.params[END_DATE_INDEX].name = END_DATE_PARAM;
	query.params[END_DATE_INDEX].param.type = INT;
	query.params[END_DATE_INDEX].param.value.int_value = range->end;

	query.params[TYPE_INDEX].name = PAYMENT_TYPE_PARAM;
	query.params[TYPE_INDEX].param.type = INT;
	query.params[TYPE_INDEX].param.value.int_value = payment_type;

	DEBUG_LOG("Getting expenses in date range [%d:%d] with payment type [%u]", range->start, range->end, payment_type);

	rc = execute_query(&query, &result);
	if (ERR_OK != rc) {
		ERR_LOG("Failed to get expenses");
		goto CLEAN_UP;
	}

	if (!result.num_rows) {
		INFO_LOG("No expenses matching query found");
		return ERR_OK;
	}

	if (NUM_EXPENSE_PARAMS != result.num_cols) {
		ERR_LOG("Received [%u] result columns but was expecting [%u]", result.num_cols, NUM_EXPENSE_PARAMS);
		goto CLEAN_UP;
	}

	result_to_expense(&result, expenses);

CLEAN_UP:

	if (query.params) {
		free(query.params);
	}

	free_results(&result);

	return rc;
}

int32_t get_expenses_in_range_with_expense_type(
	db_connection* db,
	date_range* range,
	uint32_t expense_type,
	expense_list* expenses) {

	int32_t rc;
	db_query query = {0};
	db_query_result result = {0};

	if (!db) {
		ERR_LOG("DB connection is NULL");
		return ERR_INVALID;
	}

	if (!db->handle) {
		ERR_LOG("No conection to DB available");
		return ERR_NOT_READY;
	}

	if (!expenses) {
		ERR_LOG("Expenses structure is NULL");
		return ERR_INVALID;
	}

	if (!range) {
		ERR_LOG("Date range is NULL");
		return ERR_INVALID;
	}

	query.handle = db->handle;
	query.query = SELECT_EXPENSES_IN_RANGE_WITH_EXPENSE_TYPE;
	query.num_params = NUM_RANGE_PARAMS_WITH_TYPE;
	query.params = (query_param*)malloc(sizeof(query_param) * NUM_RANGE_PARAMS_WITH_TYPE);

	if (!query.params) {
		ERR_LOG("Failed to allocate memory for params");
		return ERR_NOMEM;
	}

	query.params[START_DATE_INDEX].name = START_DATE_PARAM;
	query.params[START_DATE_INDEX].param.type = INT;
	query.params[START_DATE_INDEX].param.value.int_value = range->start;

	query.params[END_DATE_INDEX].name = END_DATE_PARAM;
	query.params[END_DATE_INDEX].param.type = INT;
	query.params[END_DATE_INDEX].param.value.int_value = range->end;

	query.params[TYPE_INDEX].name = EXPENSE_TYPE_PARAM;
	query.params[TYPE_INDEX].param.type = INT;
	query.params[TYPE_INDEX].param.value.int_value = expense_type;

	DEBUG_LOG("Getting expenses in date range [%d:%d] with expense type [%u]" , range->start, range->end, expense_type);

	rc = execute_query(&query, &result);
	if (ERR_OK != rc) {
		ERR_LOG("Failed to get expenses");
		goto CLEAN_UP;
	}

	if (!result.num_rows) {
		INFO_LOG("No expenses matching query found");
		return ERR_OK;
	}

	if (NUM_EXPENSE_PARAMS != result.num_cols) {
		ERR_LOG("Received [%u] result columns but was expecting [%u]", result.num_cols, NUM_EXPENSE_PARAMS);
		goto CLEAN_UP;
	}

	result_to_expense(&result, expenses);

CLEAN_UP:

	if (query.params) {
		free(query.params);
	}

	free_results(&result);

	return rc;
}

