/**
 * Copyright (C) 2020 Dallas Leclerc
 */
#ifndef SQL_DB_H
#define SQL_DB_H

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
enum db_type {
	INT,
	DOUBLE,
	TEXT
};

/** @struct db_value
  *
  * @details
  *		struct to for representing a value in the database
  */
struct db_value {
	enum db_type type;
	union {
		int32_t int_val;
		double double_val;
		char* string_val;
	} value;
};

/** @struct query_param
  *
  * @details
  *		struct which contains all necessary information for 
  *		a query parameter
  */
struct query_param {
	const char* name;
	struct db_value param;
};

/** @struct db_query_result
  *
  * @details
  *		Contains result from the query row will be the first index
  *		and column will be the second when accessing values.
  *
  * 	Caller is responsible for cleaning up values
  */
struct db_query_result {
	uint32_t num_rows;
	uint32_t num_cols;
	struct db_value** values;

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
 * @param[in] db_path
 *		The path to the object
 *
 * @param[out] db
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
  * @param[in] db
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
  * @param[in] query
  *		Query to execute
  *
  * @param[out] result
  *		The result of the query. Set to NULL if no result is expected
  *
  * @retval 0 if query was executed successfully
  */
int32_t execute_query(struct db_query* query, struct db_query_result* result);

#endif

