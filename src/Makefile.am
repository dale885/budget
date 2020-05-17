SUBDIRS=	\
	common	\
	sql

bin_PROGRAMS=budget_app

budget_app_SOURCES=				\
	main.c
budget_app_LDADD=				\
	sql/libsql.a				\
	common/libcommon.a			\
	-lsqlite3
budget_app_DEPENDENCIES=		\
	sql/libsql.a				\
	common/libcommon.a


