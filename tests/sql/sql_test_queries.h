/**
 * Copyright (C) 2020 Dallas Leclerc
 */

#ifndef SQL_TEST_QUERIES_H
#define SQL_TEST_QUERIES_H

#define ID_PARAM "$id_param"
#define INT_VAL_PARAM "$int_param"
#define DOUBLE_VAL_PARAM "$double_param"
#define TEXT_VAL_PARAM "$text_param"

#define CREATE_TEST_TABLE \
	"CREATE TABLE test(" \
	"id INT PRIMARY KEY NOT NULL," \
	"int_val INT NOT NULL," \
	"double_val REAL NOT NULL," \
	"text_val TEXT NOT NULL);"

#define INSERT_ROW \
	"INSERT INTO test (id, int_val, double_val, text_val) "\
	"VALUES($id_param, $int_param, $double_param, $text_param);"

#endif
