noinst_LTLIBRARIES = libbasicsecure.la

libbasicsecure_la_SOURCES = \
	basicsecure.c \
	basicsecure.h

ACLOCAL_AMFLAGS = -I m4

LDADD = libbasicsecure.la
TESTS = test_general \
	test_clear
check_PROGRAMS = $(TESTS)
