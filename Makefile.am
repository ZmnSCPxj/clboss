bin_PROGRAMS = clboss
clboss_SOURCES = \
	main.cpp

# These are the actual source files of clboss.
# We separate them here so that we can test parts of
# clboss without having to put a lot of instrumentation
# in the main file.
noinst_LTLIBRARIES = libclboss.la
libclboss_la_SOURCES = \
	Ev/Io.hpp \
	Ev/start.cpp \
	Ev/start.hpp \
	Ev/yield.cpp \
	Ev/yield.hpp \
	Util/make_unique.hpp

AM_CXXFLAGS = -Wall -Werror $(libev_CFLAGS)
LDADD = libclboss.la $(libev_LIBS)

ACLOCAL_AMFLAGS = -I m4
