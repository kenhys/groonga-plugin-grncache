AC_PREREQ(2.59)
AC_INIT([groonga-plugin-grncache], 1.0.0, [kenhys@gmail.com])

AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([src/grncache.c])
AM_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([foreign])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AC_PROG_CC

AC_PROG_LIBTOOL

AC_CHECK_HEADERS(pthread.h)
AC_CHECK_HEADERS(netdb.h)

if test "$GCC" = "yes"; then
  CFLAGS="$CFLAGS -Wall -Wextra"
  CFLAGS="$CFLAGS -Wmissing-declarations -Wmissing-prototypes"
fi

AC_ARG_ENABLE(debug,
  [AS_HELP_STRING([--enable-debug],
                  [use debug flags (default=no)])],
  [grncache_debug="$enableval"],
  [grncache_debug="no"])
if test "x$grncache_debug" != "xno"; then
  if test "$CLANG" = "yes"; then
    CFLAGS="$CFLAGS -O0 -g"
  elif test "$GCC" = "yes"; then
    CFLAGS="$CFLAGS -O0 -g3"
  fi
fi

GROONGA_REQUIRED_VERSION=3.0.8
PKG_CHECK_MODULES([GROONGA], [groonga >= ${GROONGA_REQUIRED_VERSION}])

_PKG_CONFIG(GROONGA_PLUGINS_DIR, [variable=pluginsdir],    [groonga])
_PKG_CONFIG(GROONGA,             [variable=groonga],       [groonga])

GROONGA_PLUGINS_DIR="${pkg_cv_GROONGA_PLUGINS_DIR}"
GROONGA="${pkg_cv_GROONGA}"

AC_SUBST(GROONGA_PLUGINS_DIR)
AC_SUBST(GROONGA)

AC_ARG_ENABLE(oniguruma,
  [AS_HELP_STRING([--enable-oniguruma],
                  [use Oniguruma (default=no)])],
  [grncache_oniguruma="$enableval"],
  [grncache_oniguruma="no"])
if test "x$grncache_oniguruma" != "xno"; then
  PKG_CHECK_MODULES([ONIGURUMA], [oniguruma])
  AC_DEFINE_UNQUOTED(USE_ONIGURUMA, [1], [oniguruma])
fi
AM_CONDITIONAL(WITH_ONIGURUMA, test "$grncache_oniguruma" = "yes")

# DANGER!!!: stack size
GRN_STACK_SIZE=1024
AC_ARG_WITH(stack_size,
  [AS_HELP_STRING([--with-stack-size=SIZE],
    [DANGER!!!
     This option specifies stack size. (default=$GRN_STACK_SIZE)
     Normally, you should not use this option.])],
  GRN_STACK_SIZE="$withval")
AC_DEFINE_UNQUOTED(GRN_STACK_SIZE, [$GRN_STACK_SIZE], [stack size])

grncache_pluginsdir="${GROONGA_PLUGINS_DIR}/grncache"
AC_SUBST(grncache_pluginsdir)

grn_major_version=`pkg-config --modversion groonga | cut -d'.' -f1`
grn_minor_version=`pkg-config --modversion groonga | cut -d'.' -f2`
grn_micro_version=`pkg-config --modversion groonga | cut -d'.' -f3`
AC_DEFINE_UNQUOTED(GRN_MAJOR_VERSION, [$grn_major_version], [major version])
AC_DEFINE_UNQUOTED(GRN_MINOR_VERSION, [$grn_minor_version], [minor version])
AC_DEFINE_UNQUOTED(GRN_MICRO_VERSION, [$grn_micro_version], [micro version])

AC_CONFIG_FILES([
  Makefile
  src/Makefile
])

AC_OUTPUT

echo "$PACKAGE_NAME $PACKAGE_VERSION configuration:"
echo "-----------------------"
echo "  Compiler:              ${CC}"
echo "  CFLAGS:                ${CFLAGS}"
echo "  CXXFLAGS:              ${CXXFLAGS}"
echo "  Libraries:             ${LIBS}"
echo
echo "Groonga"
echo "  CFLAGS:                ${GROONGA_CFLAGS}"
echo "  Libraries:             ${GROONGA_LIBS}"
echo "  install directory:     ${grncache_pluginsdir}"
echo "   source directory:     ${grnsrc_dir}"
echo ""
if test "$grncache_oniguruma" = "yes"; then
echo "Oniguruma"
echo "  CFLAGS:                ${ONIGURUMA_CFLAGS}"
echo "    LIBS:                ${ONIGURUMA_LIBS}"
fi
