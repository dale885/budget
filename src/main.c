/**
 * Copyright (C) 2020 Dallas Leclerc
 */

#include <stdio.h>

#include <log.h>
#include <budget_db/budget_db.h>

int main(int argc, char** argv) {

	(void)argc;

	open_log(argv[0]);

	NOTICE_LOG("Exiting");

	close_log();

	return 0;
}

