AM_CFLAGS =			\
	$(GRNINTERNAL_CFLAGS)	\
	$(ONIGURUMA_CFLAGS)	\
	$(GROONGA_CFLAGS)

AM_LDFLAGS =					\
	-avoid-version				\
	-module					\
	-no-undefined

LIBS =				\
	$(ONIGURUMA_LIBS)	\
	$(GROONGA_LIBS)

grncache_plugins_LTLIBRARIES = grncache.la

grncache_plugins_SOURCE = grncache.c
