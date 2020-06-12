
dist_doc_DATA=					\
	README.md

noinst_LIBRARIES=				\
	libcommon.a					\
	libsql.a					\
	libbudget_db.a

lib_LTLIBRARIES=				\
	libcsv_parser.la

noinst_PROGRAMS=				\
	budget_app

libcommon_a_SOURCES=			\
	src/common/budget_types.c	\
	src/common/error.c			\
	src/common/log.c

libsql_a_SOURCES=				\
	src/sql/sql_db.c

libbudget_db_a_SOURCES=	\
	src/budget_db/budget_db.c

libcsv_parser_la_SOURCES=		\
	src/parsers/csv_parser.c
libcsv_parser_la_LIBADD=		\
	-lcommon
libcsv_parser_la_DEPENDENCIES=	\
	libcommon.a

budget_app_SOURCES=				\
	src/main.c
budget_app_LDADD=				\
	-lbudget_db					\
	-lsql						\
	-lcommon					\
	-lsqlite3
budget_app_DEPENDENCIES=		\
	libbudget_db.a				\
	libsql.a					\
	libcommon.a


if ENABLE_UNIT_TESTS
noinst_PROGRAMS+=				\
	sql_test					\
	budget_db_test

sql_test_SOURCES=				\
	tests/sql/sql_test.c
sql_test_CFLAGS=				\
	$(UNITY_CFLAGS)
sql_test_LDFLAGS=				\
	$(UNITY_LDFLAGS)
sql_test_LDADD=					\
	-lsql						\
	-lcommon					\
	-lsqlite3					\
	-lunity
sql_test_DEPENDENCIES=			\
	libsql.a					\
	libcommon.a

budget_db_test_SOURCES=			\
	tests/budget_db/budget_db_test.c
budget_db_test_CFLAGS=			\
	$(UNITY_CFLAGS)
budget_db_test_LDFLAGS=			\
	$(UNITY_LDFLAGS)
budget_db_test_LDADD=			\
	-lbudget_db					\
	-lsql						\
	-lcommon					\
	-lsqlite3					\
	-lunity
budget_db_test_DEPENDENCIES=	\
	libbudget_db.a				\
	libsql.a					\
	libcommon.a

endif

ACLOCAL_AMFLAGS=-I m4

