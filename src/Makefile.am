SUBDIRS=	\
	sql

bin_PROGRAMS=budget_app

budget_app_SOURCES=				\
	main.c						\
	error.c						\
	log.c
budget_app_LDADD=				\
	sql/libsql.a				\
	-lsqlite3
budget_app_DEPENDENCIES=		\
	sql/libsql.a


