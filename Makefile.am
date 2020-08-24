bin_PROGRAMS = clboss
clboss_SOURCES = \
	main.cpp

# These are the actual source files of clboss.
# We separate them here so that we can test parts of
# clboss without having to put a lot of instrumentation
# in the main file.
noinst_LTLIBRARIES = libclboss.la
libclboss_la_SOURCES = \
	Boss/Main.cpp \
	Boss/Main.hpp \
	Ev/Io.hpp \
	Ev/start.cpp \
	Ev/start.hpp \
	Ev/yield.cpp \
	Ev/yield.hpp \
	S/Bus.cpp \
	S/Bus.hpp \
	S/Detail/Signal.hpp \
	S/Detail/SignalBase.hpp \
	Util/Str.cpp \
	Util/Str.hpp \
	Util/make_unique.hpp

# Externals
EXTRA_DIST = \
	external/jsmn/LICENSE \
	external/jsmn/jsmn.h

AM_CXXFLAGS = -Wall -Werror $(libev_CFLAGS)
AM_CPPFLAGS = -I external/jsmn
LDADD = libclboss.la $(libev_LIBS)

ACLOCAL_AMFLAGS = -I m4

AM_DEFAULT_SOURCE_EXT = .cpp
TESTS = \
	tests/boss/test_version \
	tests/ev/test_io_mem_leak \
	tests/s/test_bus
check_PROGRAMS = $(TESTS)
