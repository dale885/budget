/**
 * Copyright (C) 2020 Dallas Leclerc
 */

#include <string.h>
#include <stdlib.h>
#include <setjmp.h>

#include <budget_db.h>
#include <log.h>

#define DB_FILE_PATH "db/budget.db"
#define DIR_PERM S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH

static int32_t create_db_directory(const char* path)
{
	struct stat st;
	int32_t rc;

	if (-1 == stat(path, &st)) {
		NOTICE_LOG("Creating directory [%s]", path);

		rc = mkdir(path, DIR_PERM);
		if (0 != rc) {
			ERR_LOG("Failed to create directory [%s]: %m", path);
			return rc;
		}
	}
	return 0;
}

static void get_full_path(char** full_path, const char* dir_path)
{
	// add two for directory separator and null terminator
	int32_t path_length = strlen(dir_path) + strlen(DB_FILE_PATH) + 2;

	*full_path = (char*)malloc(sizeof(char) * path_length);
	strcpy(*full_path, dir_path);
	strcat(*full_path, "/");
	strcat(*full_path, DB_FILE_PATH);
}

int32_t open_db(const char* db_path, sqlite3** db)
{
	char* full_path;
	jmp_buf buf;
	int32_t rc;

	get_full_path(&full_path, db_path);
	NOTICE_LOG("Opening connection to db [%s]", full_path);

	rc = setjmp(buf);

	if (0 == rc) {
		rc = create_db_directory(db_path);
		if (0 != rc) {
			ERR_LOG("Failed to create DB directory [%s]:%d", db_path, rc);
			longjmp(buf, -1);
		}

		rc = sqlite3_open_v2(
			full_path,
			db,
			SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX,
			NULL);

		if (0 != rc) {
			ERR_LOG("Failed to open DB connection to [%s]:%d", full_path, rc);
			longjmp(buf, -1);
		}
	}

	free(full_path);

	return rc;
}

int32_t close_db(sqlite3* db)
{
	NOTICE_LOG("Closing database connection");

	int32_t rc = sqlite3_close(db);
	if (0 != rc)
	{
		ERR_LOG("Failed to close database connection");
		return rc;
	}

	return 0;
}

