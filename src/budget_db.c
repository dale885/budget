/**
 * Copyright (C) 2020 Dallas Leclerc
 */

#include <string.h>
#include <stdlib.h>
#include <setjmp.h>

#include <budget_db.h>
#include <log.h>

#define DB_FILE_PATH "db/budget.db"
#define DIR_PERM S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH

static int32_t create_db_directory(const char* path)
{
	struct stat st;
	int32_t rc;

	if (-1 == stat(path, &st)) {
		NOTICE_LOG("Creating directory [%s]", path);

		rc = mkdir(path, DIR_PERM);
		if (0 != rc) {
			ERR_LOG("Failed to create directory [%s]: %m", path);
			return rc;
		}
	}
	return 0;
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
		switch (param->type)
		{
			case INT:
				DEBUG_LOG("Binding value [%d] to param [%s]",
					param->param.int_val, param->name);
				rc = sqlite3_bind_int(stmt, index, param->param.int_val);
				break;
			case DOUBLE:
				DEBUG_LOG("Binding value [%f] to param [%s]",
					param->param.double_val, param->name);
				rc = sqlite3_bind_double(stmt, index, param->param.double_val);
				break;
			case TEXT:
				DEBUG_LOG("Binding value [%s] to param [%s]",
					param->param.string_val, param->name);
				rc = sqlite3_bind_text(
						stmt,
						index,
						param->param.string_val,
						-1,
						SQLITE_TRANSIENT);
				break;
		}

		if (SQLITE_OK != rc)
		{
			ERR_LOG("Failed to bind value to parameter [%s]: %d",
				param->name, rc);
			return rc;
		}
	}

	return 0;
}

int32_t open_db(const char* db_path, sqlite3** db)
{
	char* full_path;
	jmp_buf buf;
	int32_t rc;

	get_full_path(&full_path, db_path);
	NOTICE_LOG("Opening connection to db [%s]", full_path);

	rc = setjmp(buf);

	if (0 == rc) {
		rc = create_db_directory(db_path);
		if (0 != rc) {
			ERR_LOG("Failed to create DB directory [%s]:%d", db_path, rc);
			longjmp(buf, -1);
		}

		rc = sqlite3_open_v2(
			full_path,
			db,
			SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX,
			NULL);

		if (0 != rc) {
			ERR_LOG("Failed to open DB connection to [%s]:%d", full_path, rc);
			longjmp(buf, -1);
		}
	}

	free(full_path);

	return rc;
}

int32_t close_db(sqlite3* db)
{
	NOTICE_LOG("Closing database connection");

	int32_t rc = sqlite3_close(db);
	if (0 != rc)
	{
		ERR_LOG("Failed to close database connection");
		return rc;
	}

	return 0;
}

int32_t execute_query(struct db_query* query)
{
	sqlite3_stmt *stmt;
	if (!query)
	{
		ERR_LOG("query is null");
	}

	DEBUG_LOG("Preparing query [%s]", query->query);

	int32_t rc = sqlite3_prepare_v2(query->db, query->query, -1, &stmt, NULL);
	if (SQLITE_OK != rc) {
		ERR_LOG("Failed to prepare query [%s]: %d", query->query, rc);
		return rc;
	}

	if (0 < query->num_params)
	{
		rc = bind_params(stmt, query->num_params, query->params);
		if (SQLITE_OK != rc)
		{
			ERR_LOG("Failed to bind params to query [%s]", query->query);
			return rc;
		}
	}

	return 0;
}

