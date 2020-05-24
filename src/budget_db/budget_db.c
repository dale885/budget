/**
 * Copyright (C) 2020 Dallas Leclerc
 */

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sqlite3.h>

#include <budget_db/budget_db.h>
#include <sql/sql_db.h>
#include <error.h>
#include <log.h>

#define HOME_ENV "HOME"
#define DEFAULT_DB_DIR ".budget_app"

static sqlite3* g_sql = NULL;

static char* get_db_path() {
	const char* home_dir = getenv(HOME_ENV);
	if (!home_dir) {
		ERR_LOG("Failed to get home directory. [%s] not set", HOME_ENV);
		return NULL;
	}

	DEBUG_LOG("Got home dir [%s]", home_dir);
	char* db_path = (char*)malloc(
		sizeof(char) * (strlen(home_dir) + strlen(DEFAULT_DB_DIR) + 2));
	if (!db_path) {
		ERR_LOG("Failed to allocate memory for db_path");
	}

	strcpy(db_path, home_dir);
	strcat(db_path, "/");
	strcat(db_path, DEFAULT_DB_DIR);

	DEBUG_LOG("Got DB directory path [%s]", db_path);

	return db_path;
}

int32_t open_budget_db() {
	if (g_sql) {
		WARN_LOG("Connection to budget DB is already open");
		return ERR_OK;
	}

	DEBUG_LOG("Opening connection to budget DB");

	char* db_path = get_db_path();
	if (!db_path) {
		ERR_LOG("Failed to get DB path");
	}

	int32_t rc = open_db(db_path, &g_sql);
	if (ERR_OK != rc) {
		ERR_LOG("Failed to open db: [%d:%s]",
			rc, error_to_string(rc));
		free(db_path);
		return rc;
	}

	free(db_path);

	return ERR_OK;
}

int32_t close_budget_db() {
	DEBUG_LOG("Closing connection to budget DB");

	if (g_sql) {
		int32_t rc = close_db(g_sql);
		if (ERR_OK != rc) {
			ERR_LOG("Failed to close connection to budget DB: [%d:%s]",
				rc, error_to_string(rc));
			return rc;
		}
	}
	else {
		WARN_LOG("No connection to budget DB currently active");
	}

	return ERR_OK;
}

int32_t insert_expenses(expense_list* expenses) {
	if (!g_sql) {
		ERR_LOG("No conection to DB available");
		return ERR_NOT_READY;
	}

	(void)expenses;

	return ERR_KO;
}

int32_t get_expenses_in_range(date_range* range, expense_list* expenses) {
	if (!g_sql) {
		ERR_LOG("No connection to db available");
		return ERR_NOT_READY;
	}

	(void)range;
	(void)expenses;

	return ERR_KO;
}

int32_t get_expenses_in_range_with_payment_type(
	date_range* range,
	uint32_t payment_type,
	expense_list* expenses) {
	if (!g_sql) {
		ERR_LOG("No connection to db available");
		return ERR_NOT_READY;
	}

	(void)range;
	(void)payment_type;
	(void)expenses;

	return ERR_KO;
}

int32_t get_expenses_in_range_with_expense_type(
	date_range* range,
	uint32_t expense_type,
	expense_list* expenses) {
	if (!g_sql) {
		ERR_LOG("No connection to db available");
		return ERR_NOT_READY;
	}

	(void)range;
	(void)expense_type;
	(void)expenses;

	return ERR_KO;
}

