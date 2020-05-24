SUBDIRS=	\
	common	\
	sql		\
	budget_db

bin_PROGRAMS=budget_app

budget_app_SOURCES=				\
	main.c
budget_app_LDADD=				\
	budget_db/libbudget_db.a	\
	sql/libsql.a				\
	common/libcommon.a			\
	-lsqlite3
budget_app_DEPENDENCIES=		\
	budget_db/libbudget_db.a	\
	sql/libsql.a				\
	common/libcommon.a


