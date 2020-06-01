/**
 * Copyright (C) 2020 Dallas Leclerc
 */

#ifndef BUDGET_DB_QUERIES_H
#define BUDGET_DB_QUERIES_H

#define ID_COL "id"
#define AMOUNT_COL "amount"
#define DATE_COL "date"
#define PAYMENT_TYPE_COL "payment_type"
#define EXPENSE_TYPE_COL "expense_type"
#define DESCRIPTION_COL "description"

#define ID_PARAM "$id"
#define AMOUNT_PARAM "$amount"
#define DATE_PARAM "$date"
#define PAYMENT_TYPE_PARAM "$payment_type"
#define EXPENSE_TYPE_PARAM "$expense_type"
#define DESCRIPTION_PARAM "$description"

#define ID_INDEX 0
#define AMOUNT_INDEX 1
#define DATE_INDEX 2
#define PAYMENT_TYPE_INDEX 3
#define EXPENSE_TYPE_INDEX 4
#define DESCRIPTION_INDEX 5

#define NUM_EXPENSE_PARAMS 6

#define START_DATE_PARAM "$start"
#define END_DATE_PARAM "$end"


#define BEGIN_TRANSACTION "BEGIN TRANSACTION"

#define END_TRANSACTION "END TRANSACTION"

#define CREATE_EXPENSES_TABLE \
	"CREATE TABLE IF NOT EXISTS expenses(" \
	"id INT PRIMARY KEY NOT NULL," \
	"amount REAL NOT NULL," \
	"date INT NOT NULL," \
	"payment_type INT NOT NULL," \
	"expense_type INT NOT NULL," \
	"description TEXT NOT NULL);"

#define INSERT_EXPENSE \
	"INSERT INTO expenses (" \
	"id, " \
	"amount, " \
	"date, " \
	"payment_type, " \
	"expense_type, " \
	"description) "\
	"VALUES ($id, $amount, $date, $payment_type, $expense_type, $description);"

#define SELECT_EXPENSES_IN_RANGE \
	"SELECT * FROM expenses WHERE date>=$start AND date<=$end;"

#define SELECT_EXPENSES_IN_RANGE_WITH_PAYMENT_TYPE \
	"SELECT * FROM expenses WHERE date>=$start AND date<=$end AND " \
	"payment_type=$payment_type;"

#define SELECT_EXPENSES_IN_RANGE_WITH_EXPENSE_TYPE \
	"SELECT * FROM expenses WHERE date>=$start AND date<=$end AND " \
	"expense_type=$expense_type;"

#define SELECT_NUM_ROWS \
	"SELECT COUNT(*) FROM expenses;"

#endif

