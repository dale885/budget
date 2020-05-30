/**
 * Copyright (C) 2020 Dallas Leclerc
 */

#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>

#include <sql/sql_db.h>
#include <log.h>
#include <error.h>

#define DB_FILE_PATH "budget.db"
#define DB_COUNT_RESULT_QUERY "SELECT COUNT(*) FROM (%s);"
#define DIR_PERM S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH

static int32_t sqlite_error_to_error(int32_t sqlite_error)
{
	switch(sqlite_error) {
		case SQLITE_BUSY:
			return ERR_BUSY;
		default:
			return ERR_KO;
	}
}

static int32_t create_db_directory(const char* path)
{
	struct stat st;
	int32_t rc;

	if (-1 == stat(path, &st)) {
		NOTICE_LOG("Creating directory [%s]", path);

		rc = mkdir(path, DIR_PERM);
		if (0 != rc) {
			ERR_LOG("Failed to create directory [%s]: [%m]", path);
			return ERR_KO;
		}
	}
	return ERR_OK;
}

static void get_full_path(char** full_path, const char* dir_path)
{
	// add two for directory separator and null terminator
	int32_t path_length = strlen(dir_path) + strlen(DB_FILE_PATH) + 2;

	*full_path = (char*)malloc(sizeof(char) * path_length);
	if (!*full_path) {
		ERR_LOG("Failed to allocate memory for path string");
	}
	strcpy(*full_path, dir_path);
	strcat(*full_path, "/");
	strcat(*full_path, DB_FILE_PATH);
}

static int32_t bind_params(sqlite3_stmt* stmt, size_t num_params, query_param* params)
{
	int32_t index;
	int32_t rc;
	size_t i;

	if (!params) {
		ERR_LOG("params are NULL");
		return ERR_KO;
	}

	DEBUG_LOG("Binding [%u] parameters", num_params);

	for (i = 0; i < num_params; ++i)
	{
		query_param* param = &params[i];

		DEBUG_LOG("Binding param [%s]", param->name);

		index = sqlite3_bind_parameter_index(stmt, param->name);
		if (0 == index)
		{
			WARN_LOG("No parameter with name [%s] found in query", param->name);
			continue;
		}

		switch (param->param.type)
		{
			case INT:
				DEBUG_LOG("Binding value [%d] to param [%s]",
					param->param.value.int_val, param->name);
				rc = sqlite3_bind_int(stmt, index, param->param.value.int_val);
				break;
			case DOUBLE:
				DEBUG_LOG("Binding value [%f] to param [%s]",
					param->param.value.double_val, param->name);
				rc = sqlite3_bind_double(stmt, index, param->param.value.double_val);
				break;
			case TEXT:
				DEBUG_LOG("Binding value [%s] to param [%s]",
					param->param.value.string_val, param->name);
				rc = sqlite3_bind_text(
						stmt,
						index,
						param->param.value.string_val,
						-1,
						SQLITE_TRANSIENT);
				break;
		}

		if (SQLITE_OK != rc)
		{
			ERR_LOG("Failed to bind value to parameter [%s]: [%d:%s]",
				param->name, rc, sqlite3_errstr(rc));
			return sqlite_error_to_error(rc);
		}
	}

	return ERR_OK;
}

static int32_t generate_sql_statment(db_query* query, sqlite3_stmt** stmt)
{
	int32_t rc = sqlite3_prepare_v2(query->handle, query->query, -1, stmt, NULL);
	if (SQLITE_OK != rc) {
		ERR_LOG("Failed to prepare query [%s]: [%d:%s]",
			query->query, rc, sqlite3_errstr(rc));
		return sqlite_error_to_error(rc);
	}

	if (0 < query->num_params)
	{
		rc = bind_params(*stmt, query->num_params, query->params);
		if (SQLITE_OK != rc)
		{
			ERR_LOG("Failed to bind params to query [%s]", query->query);
			sqlite3_finalize(*stmt);
			*stmt = NULL;
			return sqlite_error_to_error(rc);
		}
	}

	return ERR_OK;
}

static int32_t handle_result_row(
	sqlite3_stmt* stmt,
	db_query_result* result,
	size_t row) {

	size_t num_cols;
	size_t col;
	size_t col_len;
	int32_t col_type;
	const char* col_value;

	if (!result->values) {
		ERR_LOG("Result rows have not been allocated");
		return ERR_INVALID;
	}

	num_cols = sqlite3_column_count(stmt);
	if (0 == result->num_cols) {
		result->num_cols = num_cols;
	}
	else if (num_cols != result->num_cols) {
		ERR_LOG("Columns returned [%d] does not match expected [%d]",
			num_cols, result->num_cols);
		return ERR_INVALID;
	}

	db_value* value =
		(db_value*)malloc(
			sizeof(db_value) * result->num_cols);

	if (!value) {
		ERR_LOG("Failed to allocate db value");
		return ERR_NOMEM;
	}

	for (col = 0; col < result->num_cols; ++col) {
		col_type = sqlite3_column_type(stmt, col);
		switch(col_type) {
			case SQLITE_INTEGER:
				value[col].type = INT;
				value[col].value.int_val =
					sqlite3_column_int(stmt, col);
				break;
			case SQLITE_FLOAT:
				value[col].type = DOUBLE;
				value[col].value.double_val =
					sqlite3_column_double(stmt, col);
				break;
			case SQLITE_TEXT:
				value[col].type = TEXT;
				col_value =
					(const char*)sqlite3_column_text(stmt, col);
				col_len = sqlite3_column_bytes(stmt, col) + 1;
				value[col].value.string_val =
					(char*)malloc(sizeof(char) * col_len);

				if (!value[col].value.string_val) {
					ERR_LOG(
						"Failed to allocate memory for text value");
					free(value);
					return ERR_NOMEM;
				}

				strcpy(value[col].value.string_val, col_value);
				break;
			default:
				WARN_LOG("Unsupported result type: [%d]", col_type);
				break;
		}
	}

	result->values[row] = value;

	return ERR_OK;
}

static int32_t handle_result(sqlite3_stmt* stmt, db_query_result* result)
{
	int32_t rc;
	size_t rows_processed = 0;
	bool have_retried = false;

	if (!result) {
		INFO_LOG("Result is null. Ignoring results returned from query");
	}

	do {
		rc = sqlite3_step(stmt);
		switch (rc) {
			case SQLITE_ROW:
				if (!result) {
					WARN_LOG("Result is null but results returned");
					continue;
				}
				handle_result_row(stmt, result, rows_processed);
				++rows_processed;
				break;
			case SQLITE_DONE:
				break;
			case SQLITE_BUSY:
				if (!have_retried) { 
					WARN_LOG("Database busy trying again");
					have_retried = true;
					sleep(1);
				}
				break;
			default:
				ERR_LOG("Failed to execute query: [%d:%s]",
					rc, sqlite3_errstr(rc));
				return sqlite_error_to_error(rc);
		}
	} while (
		(SQLITE_ROW == rc || SQLITE_BUSY == rc) &&
		rows_processed < result->num_rows);

	if (result &&
		rows_processed < result->num_rows) {
		WARN_LOG("Only processed [%u] rows when expecting [%u]",
			rows_processed, result->num_rows);
		result->num_rows = rows_processed;
	}

	return ERR_OK;
}
static int32_t get_num_results(db_query* query, db_query_result* result)
{
	sqlite3_stmt* stmt = NULL;
	char* count_query = NULL;
	char* original_query = NULL;
	char* split = NULL;
	int32_t query_len;
	int32_t rc;

	query_len = snprintf(
		NULL,
		0,
		DB_COUNT_RESULT_QUERY,
		query->query) + 1;
	count_query = (char*)malloc(sizeof(char) * query_len);
	if (!count_query) {
		ERR_LOG("Failed to create count query");
		rc = ERR_NOMEM;
		goto CLEAN_UP;
	}

	original_query = (char*)malloc(sizeof(char) * (strlen(query->query) + 1));
	if (!original_query) {
		ERR_LOG("Failed to allocate memory to copy query");
		rc = ERR_NOMEM;
		goto CLEAN_UP;
	}

	strcpy(original_query, query->query);
	split = strtok(original_query, ";");
	if (!split) {
		WARN_LOG("Query [%s] missing ';'",
			query->query);
		split = original_query;
	}

	snprintf(count_query, query_len, DB_COUNT_RESULT_QUERY, split);
	DEBUG_LOG("Got result count query [%s]", count_query);

	db_query sql_query = { 
		query->handle,
		count_query,
		query->num_params,
		query->params};
	rc = generate_sql_statment(&sql_query, &stmt);
	if (SQLITE_OK != rc) {
		ERR_LOG("Failed to generate sql statement: [%d:%s]",
			rc, sqlite3_errstr(rc));
		goto CLEAN_UP;
	}

	rc = sqlite3_step(stmt);
	if (SQLITE_ROW != rc) {
		ERR_LOG("Failed to get count result: [%d:%s]",
			rc, sqlite3_errstr(rc));
		if (SQLITE_OK == rc) {
			WARN_LOG("Query ran successfully but no data returned");
			rc = ERR_KO;
		}
		goto CLEAN_UP;
	}

	result->num_rows = sqlite3_column_int(stmt, 0);

	DEBUG_LOG("Got [%u] result rows", result->num_rows);

	rc = ERR_OK;

CLEAN_UP:
	if (count_query) {
		free(count_query);
	}

	if (original_query) {
		free(original_query);
	}

	if (stmt)
	{
		sqlite3_finalize(stmt);
	}

	return rc;

}

int32_t open_db(db_connection* connection)
{
	char* full_path = NULL;
	int32_t rc;
	if (!connection) {
		ERR_LOG("connection is NULL");
		return ERR_INVALID;
	}

	if (!connection->db_path) {
		ERR_LOG("DB path is NULL");
		return ERR_INVALID;
	}

	if (connection->handle) {
		ERR_LOG("DB is already open");
		return ERR_IN_USE;
	}

	get_full_path(&full_path, connection->db_path);

	NOTICE_LOG("Opening connection to db [%s]", full_path);

	rc = create_db_directory(connection->db_path);
	if (0 != rc) {
		ERR_LOG("Failed to create DB directory [%s]: [%d:%m]",
			connection->db_path, rc);
		goto CLEAN_UP;
	}

	rc = sqlite3_open_v2(
		full_path,
		(sqlite3**)&connection->handle,
		SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX,
		NULL);

	if (SQLITE_OK != rc) {
		ERR_LOG("Failed to open DB connection to [%s]: [%d:%s]",
			full_path, rc, sqlite3_errstr(rc));
		goto CLEAN_UP;
	}

CLEAN_UP:

	free(full_path);

	return rc;
}

int32_t close_db(db_connection* connection)
{
	int32_t rc;
	NOTICE_LOG("Closing database connection");

	rc = sqlite3_close(connection->handle);
	if (SQLITE_OK != rc)
	{
		ERR_LOG("Failed to close database connection: %d", rc);
		return sqlite_error_to_error(rc);
	}

	return ERR_OK;
}

int32_t execute_query(db_query* query, db_query_result* result)
{
	int32_t rc;
	if (!query) {
		ERR_LOG("query is null");
		return ERR_INVALID;
	}

	if (!query->query) {
		ERR_LOG("SQL query is NULL");
		return ERR_INVALID;
	}

	if (result) {
		DEBUG_LOG("Getting result row count");
		rc = get_num_results(query, result);
		if (0 != rc) {
			ERR_LOG("Failed to get result count");
			return rc;
		}

		DEBUG_LOG("Allocating [%u] result rows", result->num_rows)

		result->values = (db_value**)malloc(sizeof(db_value*) * result->num_rows);
		if (!result->values) {
			ERR_LOG("Failed to allocate result rows");
			return ERR_NOMEM;
		}
	}

	DEBUG_LOG("Preparing query [%s]", query->query);

	sqlite3_stmt *stmt;
	rc = generate_sql_statment(query, &stmt);
	if (ERR_OK != rc)
	{
		ERR_LOG("Failed to generate sql statement");
		return rc;
	}

	rc = handle_result(stmt, result);
	if (ERR_OK != rc)
	{
		ERR_LOG("Failed to execute query [%s]: [%d:%s]",
			query->query, rc, sqlite3_errstr(rc));
		sqlite3_finalize(stmt);
		return rc;
	}
	DEBUG_LOG("Successfully executed query");

	sqlite3_finalize(stmt);

	return ERR_OK;
}

