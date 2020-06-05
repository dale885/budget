/**
 * Copyright (C) 2020 Dallas Leclerc
 */


#include <error.h>
#include <log.h>

static const char* ERR_TO_STRING[] = {
	"Err OK",
	"Err KO",
	"Err No Memory",
	"Err Invalid",
	"Err Busy",
	"Err Not Permitted",
	"Err Not Found",
	"Err Not Ready",
	"Err In Use"
};

#define MAX_ERROR_NUMBER (sizeof(ERR_TO_STRING) / sizeof(ERR_TO_STRING[0]))

const char* error_to_string(int32_t err) {
	if (MAX_ERROR_NUMBER < (uint32_t)-err) {
		ERR_LOG("No string found for error [%d]", err);
		return "INVALID";
	}

	return ERR_TO_STRING[-err];
}

