# Run `./bootstrap` to generate the "configure" script.

# Define the package version numbers and the bug reporting link.
m4_define([CI_MAJOR], 0)
m4_define([CI_MINOR], 2)
m4_define([CI_PATCH], 5)
m4_define([CI_BUGS], [https://github.com/ci/libci/issues/new])

# Define the libtool version numbers; check the Autobook, Section 11.4.
# Bump the libtool version numbers using the following algorithm:
#   if (the current interface has not been changed):
#       CI_REVISION += 1
#   else:
#       CI_REVISION = 0
#       CI_CURRENT += 1
#       if (this release is backward compatible with the previous release):
#           CI_AGE += 1
#       else:
#           CI_AGE = 0
m4_define([CI_RELEASE], 0)
m4_define([CI_CURRENT], 2)
m4_define([CI_REVISION], 9)
m4_define([CI_AGE], 0)

# Initialize autoconf & automake.
AC_PREREQ(2.59)
AC_INIT([ci], [CI_MAJOR.CI_MINOR.CI_PATCH], [CI_BUGS])
AC_CONFIG_AUX_DIR([config])
AC_CONFIG_HEADERS([include/config.h])
AM_INIT_AUTOMAKE([1.9 foreign])

# Define macro variables for the package version numbers.
AC_DEFINE(CI_VERSION_MAJOR, CI_MAJOR, [Define the major version number.])
AC_DEFINE(CI_VERSION_MINOR, CI_MINOR, [Define the minor version number.])
AC_DEFINE(CI_VERSION_PATCH, CI_PATCH, [Define the patch version number.])
AC_DEFINE(CI_VERSION_STRING, "CI_MAJOR.CI_MINOR.CI_PATCH", [Define the version string.])

# Define substitutions for the libtool version numbers.
CI_LT_RELEASE=CI_RELEASE
CI_LT_CURRENT=CI_CURRENT
CI_LT_REVISION=CI_REVISION
CI_LT_AGE=CI_AGE
AC_SUBST(CI_LT_RELEASE)
AC_SUBST(CI_LT_CURRENT)
AC_SUBST(CI_LT_REVISION)
AC_SUBST(CI_LT_AGE)

# Note: in order to update checks, run `autoscan` and look through "configure.scan".

# Checks for programs.
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_LIBTOOL

AC_CHECK_PROG(DOXYGEN, [doxygen], [true], [false])
AM_CONDITIONAL(DOXYGEN, [test "$DOXYGEN" = true])

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T

# Define Makefiles.
AC_CONFIG_FILES([ci-0.1.pc include/Makefile src/Makefile Makefile tests/Makefile])

# Generate the "configure" script.
AC_OUTPUT
