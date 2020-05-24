/**
 * Copyright (C) 2020 Dallas Leclerc
 */

#ifndef BUDGET_DB_H
#define BUDGET_DB_H

#include <stdint.h>
#include <stdbool.h>
#include <time.h>

#define ON_DATE 0x01
#define BEFORE_DATE 0x02
#define AFTER_DATE 0x04

#define EQUALS_AMOUNT 0x01
#define LESS_THAN_AMOUNT 0x02
#define GREATER_THAN_AMOUNT 0x04

/** @struct expense
  *
  * @details
  *		Struct containing all information of an expense
  */
struct expense {
	double amount;
	uint32_t payment_type;
	uint32_t expense_type;
	time_t date;
	const char* description;
} typedef expense;

/** @struct expense_list
  *
  * @details
  *		Contains a list of expenses and the number of expenses in the
  *		list
  */
struct expense_list {
	expense* expenses;
	size_t num_expenses;
} typedef expense_list;

struct date_range {
	time_t start;
	time_t end;
} typedef date_range;

/** @brief open_budget_db
  *
  * @details
  *		Opens a connection to the underlying database for reading and
  *		writing. Caller is responsible for calling close_budget_db when
  *		finished
  *
  * @retval ERR_OK if DB opened
  */
int32_t open_budget_db();

/** @brief close_budget_db
  *
  * @details
  *		Closes a connection to the underlying database
  *
  * @retval ERR_OK if DB closed
  */
int32_t close_budget_db();

/** @brief insert_expenses
  *
  * @details
  *		Inserts the expenses into the database
  *
  * @param[in] expenses
  *		The expenses to insert
  *
  * @retval ERR_OK if all expenses added
  */
int32_t insert_expenses(expense_list* expenses);

/** @brief get_expenses_in_range
  *
  * @details
  *		Gets expenses from the specified date range
  *
  * @param[in] range
  *		Date range to check for expenses in.
  *		If start date is 0 range will get all expenses up until end date
  *		If end date is 0 will get all expenses from start date
  *		if start and end date are 0, will get all expenses
  *
  * @param[out] expenses
  *		The expenses retrieved
  *
  * @retval ERR_OK if no errors
  */
int32_t get_expenses_in_range(date_range* range, expense_list* expenses);

/** @brief get_expenses_in_range_with_payment_type
  *
  * @details
  *		Gets expenses from the specified date range with the specified
  *		payment type
  * 
  * @param[in] range
  *		Date range to check for expenses in.
  *		If start date is 0 range will get all expenses up until end date
  *		If end date is 0 will get all expenses from start date
  *		if start and end date are 0, will get all expenses
  *
  * @param[in] payment_type
  *		Payment type of expenses
  *
  * @param[out] expenses
  *		The expenses retrieved
  *
  * @retval ERR_OK if no errors
  */
int32_t get_expenses_in_range_with_payment_type(
	date_range* range,
	uint32_t payment_type,
	expense_list* expenses);

/** @brief get_expenses_in_range_with_expense_type
  *
  * @details
  *		Gets expenses from the specified date range with the specified
  *		expense type
  *
  * @param[in] range
  *		Date range to check for expenses in.
  *		If start date is 0 range will get all expenses up until end date
  *		If end date is 0 will get all expenses from start date
  *		if start and end date are 0, will get all expenses
  *
  * @param[in] expense_type
  *		Expense type of expenses
  *
  * @param[out] expenses
  *		The expenses retrieved
  *
  * @retval ERR_OK if no errors
  */
int32_t get_expenses_in_range_with_expense_type(
	date_range* range,
	uint32_t expense_type,
	expense_list* expenses);

#endif

