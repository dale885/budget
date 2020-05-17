/**
 * Copyright (C) 2020 Dallas Leclerc
 */

#include <stdio.h>

#include <log.h>
#include <sql_db.h>

int main(int argc, char** argv) {
	sqlite3* sql;

	(void)argc;

	open_log(argv[0]);

	NOTICE_LOG("Opening DB connection");

	open_db("/home/dallas/db", &sql);

	NOTICE_LOG("Closing DB connection");

	close_db(sql);

	NOTICE_LOG("Exiting");

	close_log();

	return 0;
}

