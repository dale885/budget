
bin_PROGRAMS=sql_test

sql_test_SOURCES=		\
	sql_test.c
sql_test_CFLAGS=		\
	$(UNITY_CFLAGS)
sql_test_LDFLAGS=		\
	$(UNITY_LDFLAGS)
sql_test_LDADD=			\
	sql/libsql.a			\
	common/libcommon.a		\
	-lsqlite3				\
	-lunity

