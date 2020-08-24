bin_PROGRAMS = clboss

clboss_SOURCES = \
	Ev/Io.hpp \
	Ev/start.cpp \
	Ev/start.hpp \
	Util/make_unique.hpp \
	main.cpp

AM_CXXFLAGS = -Wall -Werror $(libev_CFLAGS)
LDADD = $(libev_LIBS)

ACLOCAL_AMFLAGS = -I m4
