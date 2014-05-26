include Makefile.include

AM_CPPFLAGS += -I$(builddir)

noinst_LIBRARIES = \
  libbitcoin_server.a \
  libbitcoin_common.a \
  libratcoin_tracker.a \
  libbitcoin_cli.a

bin_PROGRAMS =

if BUILD_BITCOIND
  bin_PROGRAMS += ratcoin_tracker
endif

if BUILD_BITCOIN_CLI
  bin_PROGRAMS += bitcoin-cli
endif

SUBDIRS = . $(BUILD_QT) $(BUILD_TEST) monitor
DIST_SUBDIRS = . qt test
.PHONY: FORCE

COMMON_H = \
  common/actionHandler.h \
  common/action.h \
  common/authenticationProvider.h \
  common/communicationProtocol.h \
  common/databaseManager.h \
  common/manageNetwork.h \
  common/nodeMessages.h \
  common/periodicActionExecutor.h \
  common/ratcoinParams.h \
  common/responseVisitorInternal.h \
  common/requestHandler.h \
  common/requestResponse.h \
  common/request.h \
  common/selfNode.h \
  common/trackerResponses.h \
  common/transactionStatus.h \
  common/visitorConfigurationUtilities.h 

COMMON_CPP = \
  common/authenticationProvider.cpp \
  common/communicationProtocol.cpp \
  common/databaseManager.cpp \
  common/manageNetwork.cpp \
  common/ratcoinParams.cpp \
  common/selfNode.cpp \
  common/trackerResponses.cpp
  
# tracker core #
TRACKER_CORE_H = \
  tracker/accessFile.h \
  tracker/addressToCoins.h \
  tracker/blockInfoDatabase.h \
  tracker/clientRequestsManager.h \
  tracker/communicationProtocol.h \
  tracker/configureTrackerActionHandler.h \
  tracker/connectToTrackerRequest.h \
  tracker/connectTrackerAction.h \
  tracker/continueRequest.h \
  tracker/databaseManager.h \
  tracker/getBalanceAction.h \
  tracker/getBalanceRequest.h \
  tracker/identifyRequest.h \
  tracker/internalMediumProvider.h \
  tracker/internalOperationsMedium.h \
  tracker/originAddressScaner.h \
  tracker/nodesManager.h \
  tracker/processNetwork.h \
  tracker/segmentFileStorage.h \
  tracker/server.h \
  tracker/setResponseVisitor.h \
  tracker/simpleBuddy.h \
  tracker/trackerController.h \
  tracker/trackerControllerStates.h \
  tracker/trackerControllerEvents.h \
  tracker/transactionRecordManager.h \
  tracker/validateTransactionsAction.h \
  tracker/validateTransactionActionEvents.h \
  tracker/validateTransactionsRequest.h
  
# bitcoin core #
BITCOIN_CORE_H = \
  addrman.h \
  alert.h \
  allocators.h \
  base58.h bignum.h \
  bloom.h \
  chainparams.h \
  checkpoints.h \
  checkqueue.h \
  clientversion.h \
  coincontrol.h \
  coins.h \
  compat.h \
  core.h \
  crypter.h \
  db.h \
  hash.h \
  init.h \
  key.h \
  keystore.h \
  leveldbwrapper.h \
  limitedmap.h \
  main.h \
  mruset.h \
  netbase.h \
  net.h \
  noui.h \
  protocol.h \
  rpcclient.h \
  rpcprotocol.h \
  rpcserver.h \
  script.h \
  serialize.h \
  sync.h \
  threadsafety.h \
  tinyformat.h \
  txdb.h \
  txmempool.h \
  ui_interface.h \
  uint256.h \
  util.h \
  version.h \
  walletdb.h \
  networksParameters.h \
  wallet.h

JSON_H = \
  json/json_spirit.h \
  json/json_spirit_error_position.h \
  json/json_spirit_reader.h \
  json/json_spirit_reader_template.h \
  json/json_spirit_stream_reader.h \
  json/json_spirit_utils.h \
  json/json_spirit_value.h \
  json/json_spirit_writer.h \
  json/json_spirit_writer_template.h

obj/build.h: FORCE
	@$(MKDIR_P) $(abs_top_builddir)/src/obj
	@$(top_srcdir)/share/genbuild.sh $(abs_top_builddir)/src/obj/build.h \
	  $(abs_top_srcdir)
version.o: obj/build.h

# tracker core #
TRACKER_CORE_CPP = \
  tracker/accessFile.cpp \
  tracker/actionHandlerDefinitions.cpp \
  tracker/addressToCoins.cpp \
  tracker/blockInfoDatabase.cpp \
  tracker/connectToTrackerRequest.cpp \
  tracker/connectTrackerAction.cpp \
  tracker/continueRequest.cpp \
  tracker/clientRequestsManager.cpp \
  tracker/getBalanceAction.cpp \
  tracker/getBalanceRequest.cpp \
  tracker/identifyRequest.cpp \
  tracker/internalMediumProvider.cpp \
  tracker/internalOperationsMedium.cpp \
  tracker/nodeMedium.cpp \
  tracker/nodesManager.cpp \
  tracker/originAddressScaner.cpp \
  tracker/processNetwork.cpp \
  tracker/rpcTracker.cpp \
  tracker/segmentFileStorage.cpp \
  tracker/server.cpp \
  tracker/setResponseVisitor.cpp \
  tracker/simpleBuddy.cpp \
  tracker/trackerController.cpp \
  tracker/trackerControllerEvents.cpp \
  tracker/trackerControllerStates.cpp \
  tracker/transactionRecordManager.cpp \
  tracker/validateTransactionsAction.cpp \
  tracker/validateTransactionsRequest.cpp
  
libbitcoin_server_a_SOURCES = \
  db.cpp \
  crypter.cpp \
  addrman.cpp \
  alert.cpp \
  rpcserver.cpp \
  bloom.cpp \
  chainparams.cpp \
  checkpoints.cpp \
  coins.cpp \
  init.cpp \
  keystore.cpp \
  leveldbwrapper.cpp \
  main.cpp \
  net.cpp \
  noui.cpp \
  rpcmisc.cpp \
  rpcnet.cpp \
  rpcrawtransaction.cpp \
  txdb.cpp \
  txmempool.cpp \
  rpcdump.cpp \
  rpcwallet.cpp \
  wallet.cpp \
  walletdb.cpp \
  $(JSON_H) \
  $(BITCOIN_CORE_H)

libbitcoin_common_a_SOURCES = \
  allocators.cpp \
  chainparams.cpp \
  core.cpp \
  hash.cpp \
  key.cpp \
  netbase.cpp \
  protocol.cpp \
  rpcprotocol.cpp \
  script.cpp \
  sync.cpp \
  util.cpp \
  version.cpp \
  $(BITCOIN_CORE_H) \
  $(COMMON_H) \
  $(COMMON_CPP)

  libratcoin_tracker_a_SOURCES = \
  $(BITCOIN_CORE_H) \
  $(TRACKER_CORE_H) \
  $(TRACKER_CORE_CPP)
  
  
libbitcoin_cli_a_SOURCES = \
  rpcclient.cpp \
  $(BITCOIN_CORE_H)

nodist_libbitcoin_common_a_SOURCES = $(top_srcdir)/src/obj/build.h
#
# this repetition is temporary work around
# ratcoin_tracker binary #
ratcoin_tracker_LDADD = \
  libratcoin_tracker.a \
  libbitcoin_server.a \
  libratcoin_tracker.a \
  libbitcoin_cli.a \
  libbitcoin_common.a \
  $(LIBLEVELDB) \
  $(LIBMEMENV)
ratcoin_tracker_SOURCES = bitcoind.cpp
#

if TARGET_WINDOWS
ratcoin_tracker_SOURCES += bitcoind-res.rc
endif

AM_CPPFLAGS += $(BDB_CPPFLAGS)
ratcoin_tracker_LDADD += $(BOOST_LIBS) $(BDB_LIBS)

# bitcoin-cli binary #
bitcoin_cli_LDADD = \
  libbitcoin_cli.a \
  libbitcoin_common.a \
  $(BOOST_LIBS)
bitcoin_cli_SOURCES = bitcoin-cli.cpp
#

if TARGET_WINDOWS
bitcoin_cli_SOURCES += bitcoin-cli-res.rc
endif

# NOTE: This dependency is not strictly necessary, but without it make may try to build both in parallel, which breaks the LevelDB build system in a race
leveldb/libleveldb.a: leveldb/libmemenv.a

leveldb/%.a:
	@echo "Building LevelDB ..." && $(MAKE) -C $(@D) $(@F) CXX="$(CXX)" \
	  CC="$(CC)" PLATFORM=$(TARGET_OS) AR="$(AR)" $(LEVELDB_TARGET_FLAGS) \
	  OPT="$(CXXFLAGS) $(CPPFLAGS)"

qt/bitcoinstrings.cpp: $(libbitcoin_server_a_SOURCES) $(libbitcoin_common_a_SOURCES) $(libbitcoin_cli_a_SOURCES)
	@test -n $(XGETTEXT) || echo "xgettext is required for updating translations"
	@cd $(top_srcdir); XGETTEXT=$(XGETTEXT) share/qt/extract_strings_qt.py

CLEANFILES = leveldb/libleveldb.a leveldb/libmemenv.a *.gcda *.gcno

DISTCLEANFILES = obj/build.h

EXTRA_DIST = leveldb Makefile.include

clean-local:
	-$(MAKE) -C leveldb clean
	rm -f leveldb/*/*.gcno leveldb/helpers/memenv/*.gcno
