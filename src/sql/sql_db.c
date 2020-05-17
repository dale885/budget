/**
 * Copyright (C) 2020 Dallas Leclerc
 */

#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <setjmp.h>

#include <sql/sql_db.h>
#include <log.h>
#include <error.h>

#define DB_FILE_PATH "db/budget.db"
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
			ERR_LOG("Failed to create directory [%s]: %m", path);
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

static int32_t bind_params(sqlite3_stmt* stmt, uint32_t num_params, struct query_param* params)
{
	uint32_t i;

	DEBUG_LOG("Binding [%u] parameters", num_params);

	for (i = 0; i < num_params; ++i)
	{
		struct query_param* param = &params[i];

		DEBUG_LOG("Binding param [%s]", param->name);

		int32_t index = sqlite3_bind_parameter_index(stmt, param->name);
		if (0 == index)
		{
			WARN_LOG("No parameter with name [%s] found in query", param->name);
			continue;
		}

		int32_t rc;
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
			ERR_LOG("Failed to bind value to parameter [%s]: %d",
				param->name, rc);
			return sqlite_error_to_error(rc);
		}
	}

	return ERR_OK;
}

static int32_t generate_sql_statment(struct db_query* query, sqlite3_stmt** stmt)
{
	int32_t rc = sqlite3_prepare_v2(query->db, query->query, -1, stmt, NULL);
	if (SQLITE_OK != rc) {
		ERR_LOG("Failed to prepare query [%s]: %d", query->query, rc);
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

static int32_t handle_result(sqlite3_stmt* stmt, struct db_query_result* result)
{
	if (!result) {
		INFO_LOG("Result is null. Ignoring results returned from query");
	}

	int32_t rc;
	uint32_t rows_processed = 0;
	bool have_retried = false;
	do {
		rc = sqlite3_step(stmt);
		if (SQLITE_ROW == rc) {
			if (!result->values) {
				ERR_LOG("Result rows have not been allocated");
				return ERR_INVALID;
			}

			result->num_cols = sqlite3_column_count(stmt);
			struct db_value* value =
				(struct db_value*)malloc(
					sizeof(struct db_value) * result->num_cols);

			if (!value) {
				ERR_LOG("Failed to allocate db value");
				return ERR_NOMEM;
			}

			for (uint32_t col = 0; col < result->num_cols; ++col) {
				int32_t type = sqlite3_column_type(stmt, col);
				switch(type) {
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
						const char* col_value =
							(const char*)sqlite3_column_text(stmt, col);
						int32_t col_len = sqlite3_column_bytes(stmt, col) + 1;
						value[col].value.string_val =
							(char*)malloc(sizeof(char) * col_len);

						if (!value[col].value.string_val) {
							ERR_LOG(
								"Failed to allocate memory for text value");
							return ERR_NOMEM;
						}

						strcpy(value[col].value.string_val, col_value);
						break;
					default:
						WARN_LOG("Unsupported result type: %d", type);
						break;
				}
			}
		}
		else if (SQLITE_BUSY == rc) {
			if (!have_retried) { 
				WARN_LOG("Database busy trying again");
				have_retried = true;
				sleep(1);
			}
		}
		else {
			ERR_LOG("Failed to execute query: [%d]", rc);
			return sqlite_error_to_error(rc);
		}
	} while (SQLITE_OK != rc && ++rows_processed < result->num_rows);

	return ERR_OK;
}
static int32_t get_num_results(struct db_query* query, struct db_query_result* result)
{
	int32_t query_len = snprintf(
		NULL,
		0,
		DB_COUNT_RESULT_QUERY,
		query->query) + 1;
	char* count_query = (char*)malloc(sizeof(char) * query_len);
	if (!count_query) {
		ERR_LOG("Failed to create count query");
		return ERR_NOMEM;
	}

	sqlite3_stmt* stmt = NULL;
	jmp_buf buf;
	int32_t rc = setjmp(buf);
	if (0 != rc) {
		ERR_LOG("Failed to get the number of results: %d", rc);
		free(count_query);
		if (stmt)
		{
			sqlite3_finalize(stmt);
		}

		return rc;
	}

	struct db_query sql_query = { 
		query->db,
		count_query,
		query->num_params,
		query->params};
	rc = generate_sql_statment(&sql_query, &stmt);
	if (SQLITE_OK != rc) {
		ERR_LOG("Failed to generate sql statement: %d", rc);
		longjmp(buf, sqlite_error_to_error(rc));
	}

	rc = sqlite3_step(stmt);
	if (SQLITE_ROW != rc) {
		ERR_LOG("Failed to get count result: %d", rc);
		if (SQLITE_OK == rc) {
			WARN_LOG("Query ran successfully but no data returned");
			rc = ERR_KO;
		}
		longjmp(buf, sqlite_error_to_error(rc));
	}

	result->num_rows = sqlite3_column_int(stmt, 0);

	sqlite3_finalize(stmt);
	free (count_query);

	return ERR_OK;

}

int32_t open_db(const char* db_path, sqlite3** db)
{
	char* full_path;
	get_full_path(&full_path, db_path);

	NOTICE_LOG("Opening connection to db [%s]", full_path);

	jmp_buf buf;
	int32_t rc = setjmp(buf);

	if (0 == rc) {
		rc = create_db_directory(db_path);
		if (0 != rc) {
			ERR_LOG("Failed to create DB directory [%s]:%d", db_path, rc);
			longjmp(buf, ERR_KO);
		}

		rc = sqlite3_open_v2(
			full_path,
			db,
			SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX,
			NULL);

		if (SQLITE_OK != rc) {
			ERR_LOG("Failed to open DB connection to [%s]:%d", full_path, rc);
			longjmp(buf, sqlite_error_to_error(rc));
		}
	}

	free(full_path);

	return ERR_OK;
}

int32_t close_db(sqlite3* db)
{
	NOTICE_LOG("Closing database connection");

	int32_t rc = sqlite3_close(db);
	if (SQLITE_OK != rc)
	{
		ERR_LOG("Failed to close database connection: %d", rc);
		return sqlite_error_to_error(rc);
	}

	return ERR_OK;
}

int32_t execute_query(struct db_query* query, struct db_query_result* result)
{
	int32_t rc;
	if (!query) {
		ERR_LOG("query is null");
	}

	if (result) {
		DEBUG_LOG("Getting result row count");
		rc = get_num_results(query, result);
		if (0 != rc) {
			ERR_LOG("Failed to get result count");
			return rc;
		}

		DEBUG_LOG("Allocating [%u] result rows", result->num_rows)

		result->values = (struct db_value**)malloc(sizeof(struct db_value*) * result->num_rows);
		if (!result->values) {
			ERR_LOG("Failed to allocate result rows");
			return ERR_NOMEM;
		}
	}

	DEBUG_LOG("Preparing query [%s]", query->query);

	sqlite3_stmt *stmt;
	rc = generate_sql_statment(query, &stmt);
	if (ERR_OK != result)
	{
		ERR_LOG("Failed to generate sql statement");
		return rc;
	}

	rc = handle_result(stmt, result);
	if (ERR_OK != rc)
	{
		ERR_LOG("Failed to execute query [%s]: %d", query->query, rc);
		sqlite3_finalize(stmt);
		return rc;
	}
	DEBUG_LOG("Successfully executed query");

	sqlite3_finalize(stmt);

	return ERR_OK;
}

