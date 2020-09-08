bin_PROGRAMS = clboss
clboss_SOURCES = \
	main.cpp

# These are the actual source files of clboss.
# We separate them here so that we can test parts of
# clboss without having to put a lot of instrumentation
# in the main file.
noinst_LTLIBRARIES = libclboss.la
libclboss_la_SOURCES = \
	Boss/JsonInput.cpp \
	Boss/JsonInput.hpp \
	Boss/Main.cpp \
	Boss/Main.hpp \
	Boss/Mod/BlockTracker.cpp \
	Boss/Mod/BlockTracker.hpp \
	Boss/Mod/CommandReceiver.cpp \
	Boss/Mod/CommandReceiver.hpp \
	Boss/Mod/Initiator.cpp \
	Boss/Mod/Initiator.hpp \
	Boss/Mod/JsonOutputter.cpp \
	Boss/Mod/JsonOutputter.hpp \
	Boss/Mod/Manifester.cpp \
	Boss/Mod/Manifester.hpp \
	Boss/Mod/Rpc.cpp \
	Boss/Mod/Rpc.hpp \
	Boss/Mod/Waiter.cpp \
	Boss/Mod/Waiter.hpp \
	Boss/Mod/all.cpp \
	Boss/Mod/all.hpp \
	Boss/Msg/Begin.hpp \
	Boss/Msg/CommandRequest.hpp \
	Boss/Msg/CommandResponse.hpp \
	Boss/Msg/Init.hpp \
	Boss/Msg/JsonCin.hpp \
	Boss/Msg/JsonCout.hpp \
	Boss/Msg/ManifestCommand.hpp \
	Boss/Msg/ManifestHook.hpp \
	Boss/Msg/ManifestNotification.hpp \
	Boss/Msg/Manifestation.hpp \
	Boss/Msg/Network.hpp \
	Boss/Msg/Notification.hpp \
	Boss/Shutdown.hpp \
	Boss/concurrent.cpp \
	Boss/concurrent.hpp \
	Boss/log.cpp \
	Boss/log.hpp \
	Boss/open_rpc_socket.cpp \
	Boss/open_rpc_socket.hpp \
	Ev/Io.hpp \
	Ev/ThreadPool.cpp \
	Ev/ThreadPool.hpp \
	Ev/concurrent.cpp \
	Ev/concurrent.hpp \
	Ev/start.cpp \
	Ev/start.hpp \
	Ev/yield.cpp \
	Ev/yield.hpp \
	Jsmn/Detail/EndAdvancer.cpp \
	Jsmn/Detail/EndAdvancer.hpp \
	Jsmn/Detail/ParseResult.hpp \
	Jsmn/Detail/Str.cpp \
	Jsmn/Detail/Str.hpp \
	Jsmn/Detail/Token.cpp \
	Jsmn/Detail/Token.hpp \
	Jsmn/Detail/Type.hpp \
	Jsmn/Object.cpp \
	Jsmn/Object.hpp \
	Jsmn/Parser.cpp \
	Jsmn/Parser.hpp \
	Jsmn/jsonify_string.cpp \
	Jsmn/jsonify_string.hpp \
	Json/Out.hpp \
	Net/Fd.cpp \
	Net/Fd.hpp \
	S/Bus.cpp \
	S/Bus.hpp \
	S/Detail/Signal.hpp \
	S/Detail/SignalBase.hpp \
	Stats/RunningMean.cpp \
	Stats/RunningMean.hpp \
	Util/Rw.cpp \
	Util/Rw.hpp \
	Util/Str.cpp \
	Util/Str.hpp \
	Util/make_unique.hpp

# Externals
EXTRA_DIST = \
	external/jsmn/LICENSE \
	external/jsmn/jsmn.h

AM_CXXFLAGS = -Wall -Werror $(PTHREAD_CFLAGS) $(libev_CFLAGS)
AM_CPPFLAGS = -I external/jsmn
LDADD = libclboss.la $(PTHREAD_LIBS) $(libev_LIBS)

ACLOCAL_AMFLAGS = -I m4

AM_DEFAULT_SOURCE_EXT = .cpp
TESTS = \
	tests/boss/test_rpc \
	tests/boss/test_version \
	tests/ev/test_concurrent_simple \
	tests/ev/test_io_mem_leak \
	tests/json/test_out_simple \
	tests/s/test_bus \
	tests/stats/test_running_mean
check_PROGRAMS = $(TESTS)
