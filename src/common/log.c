/**
 * Copyright (C) 2020 Dallas Leclerc
 */

#include <log.h>

void open_log(const char* name)
{
	openlog(name, LOG_ODELAY | LOG_PID, LOG_USER);
}

void close_log()
{
	closelog();
}

