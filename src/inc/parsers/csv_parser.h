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

/** @struct csv_data
  *
  * @details
  *		Struct for containing parsed CSV data
  *		CSV data is accessed from data by data[row][column]
  */
struct csv_data {
	size_t num_rows;
	size_t num_cols;
	char*** data;
} typedef csv_data;

/** @brief parse_csv_file
  *
  * @details
  *		Opens and parses a CSV file data must be freed by using
  *		free_csv_data when caller is finished with data
  *
  * @param [in] filename
  *		filename of CSV file
  *
  * @param [in] has_header
  *		Whether or not the file has a header
  *
  * @param [out] data
  *		Parsed CSV data
  */
int32_t parse_csv_file(const char* filename, bool has_header, csv_data* data);

/** @brief free_csv_data
  *
  * @details
  *		Helper function to clean up CSV data struct
  */
void free_csv_data(csv_data* restrict data);

#endif

