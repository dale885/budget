/**
 * Copyright (C) 2020 Dallas Leclerc
 */

#ifndef LOG_H_
#define LOG_H_

#include <syslog.h>
#include <stdint.h>


void write_log(int32_t level, const char* msg, const char* file, int32_t line, ...);

void open_log(const char* name);

void close_log();

/* Macros to easily handle logging */

#define EMERG_LOG(msg, ...) write_log(LOG_EMERG, msg, __FILE__, __LINE__, ##__VA_ARGS__);
#define ALERT_LOG(msg, ...) write_log(LOG_ALERT, msg, __FILE__, __LINE__, ##__VA_ARGS__);
#define CRIT_LOG(msg, ...) write_log(LOG_CRIT, msg, __FILE__, __LINE__, ##__VA_ARGS__);
#define ERR_LOG(msg, ...) write_log(LOG_ERR, msg, __FILE__, __LINE__, ##__VA_ARGS__);
#define WARN_LOG(msg, ...) write_log(LOG_WARNING, msg, __FILE__, __LINE__, ##__VA_ARGS__);
#define NOTICE_LOG(msg, ...) write_log(LOG_NOTICE, msg, __FILE__, __LINE__, ##__VA_ARGS__);
#define INFO_LOG(msg, ...) write_log(LOG_INFO, msg, __FILE__, __LINE__, ##__VA_ARGS__);
#define DEBUG_LOG(msg, ...) write_log(LOG_DEBUG, msg, __FILE__, __LINE__, ##__VA_ARGS__);

#endif

