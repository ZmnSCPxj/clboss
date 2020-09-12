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
	Boss/Mod/ChannelFinderByPopularity.cpp \
	Boss/Mod/ChannelFinderByPopularity.hpp \
	Boss/Mod/ConnectFinderByDns.cpp \
	Boss/Mod/ConnectFinderByDns.hpp \
	Boss/Mod/ConnectFinderByHardcode.cpp \
	Boss/Mod/ConnectFinderByHardcode.hpp \
	Boss/Mod/Connector.cpp \
	Boss/Mod/Connector.hpp \
	Boss/Mod/InitialConnect.cpp \
	Boss/Mod/InitialConnect.hpp \
	Boss/Mod/Initiator.cpp \
	Boss/Mod/Initiator.hpp \
	Boss/Mod/JsonOutputter.cpp \
	Boss/Mod/JsonOutputter.hpp \
	Boss/Mod/ListpeersAnnouncer.cpp \
	Boss/Mod/ListpeersAnnouncer.hpp \
	Boss/Mod/Manifester.cpp \
	Boss/Mod/Manifester.hpp \
	Boss/Mod/NeedsConnectSolicitor.cpp \
	Boss/Mod/NeedsConnectSolicitor.hpp \
	Boss/Mod/Reconnector.cpp \
	Boss/Mod/Reconnector.hpp \
	Boss/Mod/Rpc.cpp \
	Boss/Mod/Rpc.hpp \
	Boss/Mod/Timers.cpp \
	Boss/Mod/Timers.hpp \
	Boss/Mod/Waiter.cpp \
	Boss/Mod/Waiter.hpp \
	Boss/Mod/all.cpp \
	Boss/Mod/all.hpp \
	Boss/Msg/Begin.hpp \
	Boss/Msg/Block.hpp \
	Boss/Msg/CommandRequest.hpp \
	Boss/Msg/CommandResponse.hpp \
	Boss/Msg/Init.hpp \
	Boss/Msg/JsonCin.hpp \
	Boss/Msg/JsonCout.hpp \
	Boss/Msg/ListpeersResult.hpp \
	Boss/Msg/ManifestCommand.hpp \
	Boss/Msg/ManifestHook.hpp \
	Boss/Msg/ManifestNotification.hpp \
	Boss/Msg/Manifestation.hpp \
	Boss/Msg/NeedsConnect.hpp \
	Boss/Msg/Network.hpp \
	Boss/Msg/Notification.hpp \
	Boss/Msg/ProposeChannelCandidates.hpp \
	Boss/Msg/ProposeConnectCandidates.hpp \
	Boss/Msg/RequestConnect.hpp \
	Boss/Msg/ResponseConnect.hpp \
	Boss/Msg/SolicitChannelCandidates.hpp \
	Boss/Msg/SolicitConnectCandidates.hpp \
	Boss/Msg/TaskCompletion.hpp \
	Boss/Msg/Timer10Minutes.hpp \
	Boss/Msg/TimerRandomDaily.hpp \
	Boss/Msg/TimerRandomHourly.hpp \
	Boss/Shutdown.hpp \
	Boss/concurrent.cpp \
	Boss/concurrent.hpp \
	Boss/log.cpp \
	Boss/log.hpp \
	Boss/random_engine.cpp \
	Boss/random_engine.hpp \
	Boss/open_rpc_socket.cpp \
	Boss/open_rpc_socket.hpp \
	DnsSeed/Detail/decode_bech32_node.cpp \
	DnsSeed/Detail/decode_bech32_node.hpp \
	DnsSeed/Detail/parse_dig_srv.cpp \
	DnsSeed/Detail/parse_dig_srv.hpp \
	DnsSeed/get.cpp \
	DnsSeed/get.hpp \
	Ev/Io.hpp \
	Ev/ThreadPool.cpp \
	Ev/ThreadPool.hpp \
	Ev/concurrent.cpp \
	Ev/concurrent.hpp \
	Ev/map.hpp \
	Ev/runcmd.cpp \
	Ev/runcmd.hpp \
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
	Ln/NodeId.cpp \
	Ln/NodeId.hpp \
	Net/Fd.cpp \
	Net/Fd.hpp \
	S/Bus.cpp \
	S/Bus.hpp \
	S/Detail/Signal.hpp \
	S/Detail/SignalBase.hpp \
	Sqlite3.hpp \
	Sqlite3/Db.cpp \
	Sqlite3/Db.hpp \
	Sqlite3/Detail/binds.cpp \
	Sqlite3/Detail/binds.hpp \
	Sqlite3/Detail/columns.cpp \
	Sqlite3/Detail/columns.hpp \
	Sqlite3/Query.cpp \
	Sqlite3/Query.hpp \
	Sqlite3/Result.cpp \
	Sqlite3/Result.hpp \
	Sqlite3/Tx.cpp \
	Sqlite3/Tx.hpp \
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

AM_CXXFLAGS = -Wall -Werror $(PTHREAD_CFLAGS) $(libev_CFLAGS) $(SQLITE3_CFLAGS)
AM_CPPFLAGS = -I $(top_srcdir)/external/jsmn
LDADD = libclboss.la $(PTHREAD_LIBS) $(libev_LIBS) $(SQLITE3_LDFLAGS)

ACLOCAL_AMFLAGS = -I m4

AM_DEFAULT_SOURCE_EXT = .cpp
TESTS = \
	tests/boss/test_needsconnectsolicitor \
	tests/boss/test_rpc \
	tests/boss/test_version \
	tests/dnsseed/test_decode_bech32_node \
	tests/dnsseed/test_get \
	tests/dnsseed/test_parse_dig_srv \
	tests/ev/test_concurrent_simple \
	tests/ev/test_io_mem_leak \
	tests/ev/test_map \
	tests/ev/test_runcmd \
	tests/ev/test_throw_in_then \
	tests/json/test_out_simple \
	tests/ln/test_nodeid \
	tests/s/test_bus \
	tests/sqlite3/test_sqlite3 \
	tests/stats/test_running_mean
check_PROGRAMS = $(TESTS)
