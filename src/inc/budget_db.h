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

#endif

