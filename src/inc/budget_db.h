/**
 * Copyright (C) 2020 Dallas Leclerc
 */
#ifndef BUDGET_DB_H
#define BUDGET_DB_H

#include <sys/stat.h>
#include <sys/types.h>
#include <stdint.h>
#include <sqlite3.h>

#include <log.h>

/** @enum db_param_type
  *
  * @details
  *		enum for the different parameters type supported for query
  */
enum query_param_type {
	INT,
	DOUBLE,
	TEXT
};

/** @struct query_param
  *
  * @details
  *		struct which contains all necessary information for 
  *		a query parameter
  */
struct query_param {
	const char* name;
	enum query_param_type type;
	union {
		int32_t int_val;
		double double_val;
		const char* string_val;
	} param;
};

struct db_query {
	sqlite3* db;
	const char* query;
	uint32_t num_params;
	struct query_param* params;
};

/** @brief open_db
 *
 * @details
 *		Opens connection to database at the path provided
 *
 * @param db_path
 *		The path to the object
 *
 * @param db
 *		Pointer to a pointer to sqlite3 object
 *
 * @retval 0 if connection created successfully
 */
int32_t open_db(const char* db_path, sqlite3** db);

/** @brief close_db
  *
  * @details
  *		Closes connection to database
  *
  * @param db
  *		Database connection to close
  *
  * @retval 0 if database connection is successfully closed
  */
int32_t close_db(sqlite3* db);

/** @brief execute_query
  *
  * @details
  *		Executes database query
  *
  * @param query
  *		Query to execute
  *
  * @retval 0 if query was executed successfully
  */
int32_t execute_query(struct db_query* query);

#endif

