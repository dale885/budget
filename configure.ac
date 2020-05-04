AC_INIT([Budget App], [0.1.0], [dale885])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_PROG_CC
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([
	Makefile
	src/Makefile
])

#Set common flags for app
AC_SUBST([AM_CFLAGS], ["-g -O2 -Wall -Werror -Iinc"])
AC_SUBST([AM_LDFLAGS], ["-lsqlite3"])

#Check for required libs
AC_CHECK_HEADER([sqlite3.h], HAVE_SQLITE_HEADER="true",
	AC_MSG_ERROR([sqlite not installed on systems]))
AC_CHECK_LIB([sqlite3], [sqlite3_exec], HAVE_SQLITE_LIB="yes",
	AC_MSG_ERROR([sqlite library not installed on system]))

AC_OUTPUT

