/**
 * Copyright (C) 2020 Dallas Leclerc
 */

#ifndef BUDGET_TYPES_H
#define BUDGET_TYPES_H

#include <stdint.h>
#include <unistd.h>

/** @enum budget_value_type
  *
  * @details
  *		enum for the different value types
  */
enum budget_value_type {
	INT,
	DOUBLE,
	STRING
} typedef budget_value_type;

/** @struct budget_value
  *
  * @details
  *		Struct to hold a generic value
  */
struct budget_value {
	budget_value_type type;
	union {
		int32_t int_value;
		double double_value;
		char* string_value;
	} value;
} typedef budget_value;

/** @brief free_value
  *
  * @details
  *		Helper function for freeing string value
  *
  * @param [in] value
  *		The value to free
  */
void free_value(budget_value* restrict value);

/** @brief free_values
  *
  * @details
  *		Helper function for freeing string values
  *
  * @param [in] values
  *		The values to free
  *
  * @param [in] num_values
  *		The number of values to free
  */
void free_values(budget_value* restrict values, size_t num_values);

#endif

