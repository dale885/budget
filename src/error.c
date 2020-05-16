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
	"Err Not Found"
};

#define MAX_ERROR_NUMBER 6

const char* error_to_string(int32_t err)
{
	if (MAX_ERROR_NUMBER < -err) {
		ERR_LOG("No string found for error [%d]", err);
		return "";
	}

	return ERR_TO_STRING[-err];
}

