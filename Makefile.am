
dist_doc_DATA=					\
	README.md

noinst_LIBRARIES=				\
	libcommon.a					\
	libsql.a					\
	libbudget_db.a

bin_PROGRAMS=					\
	budget_app

libcommon_a_SOURCES=			\
	error.c						\
	log.c

libsql_a_SOURCES=				\
	sql_db.c

libbudget_db_a_SOURCES=	\
	budget_db.c

budget_app_SOURCES=				\
	main.c
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
bin_PROGRAMS+=					\
	sql_test					\
	budget_db_test

sql_test_SOURCES=			\
	sql_test.c
sql_test_CFLAGS=			\
	$(UNITY_CFLAGS)
sql_test_LDFLAGS=			\
	$(UNITY_LDFLAGS)
sql_test_LDADD=				\
	-lsql					\
	-lcommon				\
	-lsqlite3				\
	-lunity

budget_db_test_SOURCES=		\
	budget_db_test.c
budget_db_test_CFLAGS=		\
	$(UNITY_CFLAGS)
budget_db_test_LDFLAGS=		\
	$(UNITY_LDFLAGS)
budget_db_test_LDADD=		\
	-lbudget_db				\
	-lsql					\
	-lcommon				\
	-lsqlite3				\
	-lunity

endif

