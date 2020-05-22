/**
 * Copyright (C) 2020 Dallas Leclerc
 */

#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>

#include <log.h>

#define FILENAME_AND_LINE_FORMATTING "[%s:%d]:"

void open_log(const char* name)
{
	openlog(name, LOG_ODELAY | LOG_PID, LOG_USER);
}

void close_log()
{
	closelog();
}

void write_log(int32_t level, const char* msg, const char* file, int32_t line, ...)
{

	char* message = NULL;

	int32_t len = snprintf(
		NULL,
		0,
		FILENAME_AND_LINE_FORMATTING,
		file,
		line);
	len += strlen(msg) + 1; 
	message = (char*)malloc(sizeof(char) * len);
	if (!message)
	{
		syslog(LOG_CRIT, "Unable to allocate buffer for log message");
		return;
	}

	int32_t rc = snprintf(
		message,
		len,
		FILENAME_AND_LINE_FORMATTING,
		file,
		line);
	if (rc >= len)
	{
		syslog(LOG_CRIT, "Unable to format log message");
		free(message);
		return;
	}

	strcat(message, msg);

	va_list args;
	va_start(args, line);

	vsyslog(level, message, args);

	va_end(args);

	free(message);
}

