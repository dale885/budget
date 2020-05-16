/**
 * Copyright (C) 2020 Dallas Leclerc
 */

#include <stdint.h>

#ifndef ERROR_H
#define ERROR_H

#define ERR_OK 0
#define ERR_KO -1
#define ERR_NOMEM -2
#define ERR_INVALID -3
#define ERR_BUSY -4
#define ERR_NOT_PERMITTED -5
#define ERR_NOT_FOUND -6

const char* error_to_string(int32_t err);

#endif

