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
#define START_DATE_PARAM "$start"
#define END_DATE_PARAM = "$end"

#define CREATE_EXPENSES_TABLE \
	"CREATE TABLE expenses(" \
	"id INT PRIMARY KEY NOT NULL," \
	"amount REAL NOT NULL," \
	"date INT NOT NULL," \
	"payment_type INT NOT NULL," \
	"expense_type INT NOT NULL," \
	"description TEXT NOT NULL);"

#define CHECK_EXPENSES_TABLE_EXISTS \
	"SELECT COUNT(*) FROM sqlite_master WHERE type='table' " \
	"AND name='expenses';"

#define INSERT_EXPENSE \
	"INSERT INTO expenses (" \
	"id, " \
	"amount, " \
	"date, " \
	"payment_type, " \
	"expense_type, " \
	"payment_type) "\
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

