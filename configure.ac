AC_PREREQ(2.59)
AC_INIT([groonga-plugin-grncache], 1.0.0, [kenhys@gmail.com])

AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([src/grncache.c])
AM_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([foreign])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AC_PROG_CC

AC_PROG_LIBTOOL

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

GROONGA_REQUIRED_VERSION=4.0.0
PKG_CHECK_MODULES([GROONGA], [groonga >= ${GROONGA_REQUIRED_VERSION}])

_PKG_CONFIG(GROONGA_PLUGINS_DIR, [variable=pluginsdir],    [groonga])
_PKG_CONFIG(GROONGA,             [variable=groonga],       [groonga])

GROONGA_PLUGINS_DIR="${pkg_cv_GROONGA_PLUGINS_DIR}"
GROONGA="${pkg_cv_GROONGA}"

AC_SUBST(GROONGA_PLUGINS_DIR)
AC_SUBST(GROONGA)

grncache_pluginsdir="${GROONGA_PLUGINS_DIR}/grncache"
AC_SUBST(grncache_pluginsdir)

AC_ARG_WITH(groonga-source-dir,
  [AS_HELP_STRING([--with-groonga-source-dir],
                  [specify Groonga source directory (default=empty)])],
  [grnsrc_dir="$withval"])
GRNINTERNAL_CFLAGS="-I${grnsrc_dir}/lib"
AC_SUBST(GRNINTERNAL_CFLAGS)


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
