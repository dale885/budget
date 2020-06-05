/**
 * Copyright (C) 2020 Dallas Leclerc
 */
#ifndef SQL_DB_H
#define SQL_DB_H

#include <sys/stat.h>
#include <sys/types.h>
#include <stdint.h>
#include <sqlite3.h>

#include <budget_types.h>
#include <log.h>

/** @struct query_param
  *
  * @details
  *		struct which contains all necessary information for 
  *		a query parameter
  */
struct query_param {
	const char* name;
	budget_value param;
} typedef query_param;

/** @struct db_query_result
  *
  * @details
  *		Contains result from the query row will be the first index
  *		and column will be the second when accessing values.
  *
  * 	Caller is responsible for cleaning up values
  */
struct db_query_result {
	size_t num_rows;
	size_t num_cols;
	budget_value** values;

} typedef db_query_result;

/** @struct db_query
  *
  * @details
  *		Contains all information required to make a query
  *		to the data base
  */
struct db_query {
	sqlite3* handle;
	const char* query;
	size_t num_params;
	query_param* params;
} typedef db_query;

/** @struct db_connection
  *
  * @details
  *		Used when interacting with the database
  */
struct db_connection {
	sqlite3* handle;
	char* db_path;
} typedef db_connection;

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
int32_t open_db(db_connection* connection);

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
int32_t close_db(db_connection* connection);

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
int32_t execute_query(db_query* query, db_query_result* result);

/** @brief free_results
  *
  * @details
  *		Helper to free results once they are no longer needed
  *
  * @param[in] results
  *		The results to free
  */
void free_results(db_query_result* restrict results);

/** @brief free_params
  *
  * @details
  *		Helper to free and text params in param list
  */
void free_params(query_param* restrict params, uint32_t num_params);

#endif

