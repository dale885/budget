/*
 * Copyright (C) 2020 Dallas Leclerc
 */

#ifndef CSV_PARSER_H
#define CSV_PARSER_H


#include <stdint.h>
#include <unistd.h>
#include <stdbool.h>

#include <budget_types.h>
#include <error.h>

/** @typedef csv_handle
  *
  * @details
  *		Opaque structure for parsing CSV files iteratively
  */
typedef struct csv_parser* csv_handle;

/** @struct csv_data
  *
  * @details
  *		Struct for containing parsed CSV data
  *		CSV data is accessed from data by data[column]
  */
struct csv_data {
	size_t num_cols;
	char** data;
} typedef csv_data;

/** @enum line_end
  *
  * @details
  *		enum to describe what line ending is used for the CSV file
  */
enum line_end {
	unix_line_end,
	dos_line_end,
	mac_line_end
} typedef line_end;

/** @brief csv_free_data
  *
  * @details
  *		Helper function to clean up CSV data struct
  */
void csv_free_data(csv_data* restrict data);

/** @brief csv_opem
  *
  * @details
  *		Opens a CSV file for parsing
  *
  * @param [in] filename
  *		The CSV file to be parsed
  *
  * @param [in] has_header
  *		Whether or not the file has a header
  *
  * @param [in] line_ending
  *		The type of line ending the file has
  *		if NULL will assume unix line endings
  *
  *
  * @retval csv_handle on success
  *
  * @retval NULL on failure
  */
csv_handle csv_open(const char* filename, const bool has_header, const line_end* const restrict line_ending); 

/** @brief csv_get_next_row
  *
  * @details
  *		Gets the next row from the CSV file
  *
  * @param [in] handle
  *		The handle for the CSV file to get next row from
  *
  * @retval Pointer to csv_data structure. NULL if no more rows
  */
csv_data* csv_get_next_row(csv_handle restrict handle);

/** @brief csv_close
  *
  * @details
  *		Closes the CSV file
  *
  * @param [in] handle
  *		The handle to the file to be closed
  */
void csv_close(csv_handle restrict handle);

#endif

