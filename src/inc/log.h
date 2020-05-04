#include <syslog.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

#ifndef LOG_H_
#define LOG_H_

/* Macros to easily handle logging */

#define EMERG_LOG(msg, ...) write_log(LOG_EMERG, msg, ##__VA_ARGS__);
#define ALERT_LOG(msg, ...) write_log(LOG_ALERT, msg, ##__VA_ARGS__);
#define CRIT_LOG(msg, ...) write_log(LOG_CRIT, msg, ##__VA_ARGS__);
#define ERR_LOG(msg, ...) write_log(LOG_ERR, msg, ##__VA_ARGS__);
#define WARN_LOG(msg, ...) write_log(LOG_WARNING, msg, ##__VA_ARGS__);
#define NOTICE_LOG(msg, ...) write_log(LOG_NOTICE, msg, ##__VA_ARGS__);
#define INFO_LOG(msg, ...) write_log(LOG_INFO, msg, ##__VA_ARGS__);
#define DEBUG_LOG(msg, ...) write_log(LOG_DEBUG, msg, ##__VA_ARGS__);

#define FILENAME_AND_LINE_FORMATTING "[%s:%d]:"

void open_log(const char* name)
{
	openlog(name, LOG_ODELAY | LOG_PID, LOG_USER);
}

void close_log()
{
	closelog();
}

void write_log(int32_t level, const char* msg, ...)
{

	char* message = NULL;

	uint32_t len = snprintf(
		NULL,
		0,
		FILENAME_AND_LINE_FORMATTING,
		__FILE__,
		__LINE__);
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
		__FILE__,
		__LINE__);
	if (rc >= len)
	{
		syslog(LOG_CRIT, "Unable to format log message");
		return;
	}
	strcat(message, msg);

	va_list args;
	va_start(args, msg);

	vsyslog(level, message, args);

	va_end(args);
}

#endif
