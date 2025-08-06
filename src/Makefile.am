AM_CPPFLAGS = -I$(top_srcdir)/include -Wall
lib_LTLIBRARIES = libci.la
libci_la_SOURCES = ci_private.h api.c reader.c scanner.c parser.c loader.c writer.c emitter.c dumper.c
libci_la_LDFLAGS = -no-undefined -release $(CI_LT_RELEASE) -version-info $(CI_LT_CURRENT):$(CI_LT_REVISION):$(CI_LT_AGE)
