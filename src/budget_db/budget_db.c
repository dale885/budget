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

int32_t open_budget_db(db_connection* db) {
	int32_t rc;

	if (!db) {
		ERR_LOG("DB db is NULL");
		return ERR_INVALID;
	}

	if (db->handle) {
		WARN_LOG("Connection to budget DB is already open");
		return ERR_OK;
	}

	DEBUG_LOG("Opening db to budget DB");

	rc = open_db(db);
	if (ERR_OK != rc) {
		ERR_LOG("Failed to open db: [%d:%s]",
			rc, error_to_string(rc));
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
		WARN_LOG("No db to budget DB currently active");
	}

	return ERR_OK;
}

int32_t insert_expenses(db_connection* db, expense_list* expenses) {
	if (!db) {
		ERR_LOG("DB connection is NULL");
		return ERR_INVALID;
	}

	if (!db->handle) {
		ERR_LOG("No conection to DB available");
		return ERR_NOT_READY;
	}

	(void)expenses;

	return ERR_KO;
}

int32_t get_expenses_in_range(
	db_connection* db,
	date_range* range,
	expense_list* expenses) {

	if (!db) {
		ERR_LOG("DB connection is NULL");
		return ERR_INVALID;
	}

	if (!db->handle) {
		ERR_LOG("No conection to DB available");
		return ERR_NOT_READY;
	}

	(void)range;
	(void)expenses;

	return ERR_KO;
}

int32_t get_expenses_in_range_with_payment_type(
	db_connection* db,
	date_range* range,
	uint32_t payment_type,
	expense_list* expenses) {

	if (!db) {
		ERR_LOG("DB connection is NULL");
		return ERR_INVALID;
	}

	if (!db->handle) {
		ERR_LOG("No conection to DB available");
		return ERR_NOT_READY;
	}

	(void)range;
	(void)payment_type;
	(void)expenses;

	return ERR_KO;
}

int32_t get_expenses_in_range_with_expense_type(
	db_connection* db,
	date_range* range,
	uint32_t expense_type,
	expense_list* expenses) {

	if (!db) {
		ERR_LOG("DB connection is NULL");
		return ERR_INVALID;
	}

	if (!db->handle) {
		ERR_LOG("No conection to DB available");
		return ERR_NOT_READY;
	}

	(void)range;
	(void)expense_type;
	(void)expenses;

	return ERR_KO;
}

