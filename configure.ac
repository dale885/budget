AC_INIT([Budget App], [0.1.0], [dale885])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_PROG_CC
AC_PROG_RANLIB
AM_PROG_AR
AC_CONFIG_FILES([
	Makefile
	src/Makefile
	src/common/Makefile
	src/sql/Makefile
	tests/Makefile
	tests/sql/Makefile
])

#Set common flags for app
AC_SUBST([AM_CFLAGS], ["-g -O2 -Wall -Werror -I$PWD/src/inc -std=gnu17"])

#Check for required libs
AC_CHECK_HEADER([sqlite3.h], HAVE_SQLITE_HEADER="true",
	AC_MSG_ERROR([sqlite not installed on systems]))
AC_CHECK_LIB([sqlite3], [sqlite3_exec], HAVE_SQLITE_LIB="yes",
	AC_MSG_ERROR([sqlite library not installed on system]))

AC_ARG_ENABLE(
	[unit-tests],
	[AS_HELP_STRING([--enable-unit-tests], [Build unit tests])],
	[enable_unit_tests=yes])

AM_CONDITIONAL([WITH_UNIT_TESTS], [test "x$enable_unit_tests" = xyes])

AS_IF([test "x$enable_unit_tests" = xyes],
	AC_SUBST([UNITY_CFLAGS], ["-DUNITY_INCLUDE_DOUBLE -I$PWD/src/inc -I$PWD/Unity/src -L$PWD/Unity/build -L$PWD/src/sql -L$PWD/src/common"]))
AC_OUTPUT

