include Makefile.include

AM_CPPFLAGS += -I$(builddir)

noinst_LIBRARIES = \
  libbitcoin_server.a \
  libbitcoin_common.a \
  libdims_tracker.a \
  libbitcoin_cli.a

bin_PROGRAMS =

if BUILD_BITCOIND
  bin_PROGRAMS += dims_tracker
endif

if BUILD_BITCOIN_CLI
  bin_PROGRAMS += bitcoin-cli
endif

SUBDIRS = . $(BUILD_QT) $(BUILD_TEST) monitor seed
DIST_SUBDIRS = . qt test
.PHONY: FORCE
	
COMMON_H = \
  common/actionHandler.h \
  common/action.h \
  common/authenticationProvider.h \
  common/communicationProtocol.h \
  common/commonEvents.h \
  common/commonResponses.h \
  common/databaseManager.h \
  common/filters.h \
  common/identifyRequest.h \
  common/manageNetwork.h \
  common/nodeMedium.h \
  common/nodeMessages.h \
  common/nodesManager.h \
  common/periodicActionExecutor.h \
  common/dimsParams.h \
  common/responseVisitorInternal.h \
  common/requestHandler.h \
  common/requestResponse.h \
  common/request.h \
  common/selfNode.h \
  common/transactionStatus.h \
  common/visitorConfigurationUtilities.h 

COMMON_CPP = \
  common/authenticationProvider.cpp \
  common/communicationProtocol.cpp \
  common/commonResponses.cpp \
  common/databaseManager.cpp \
  common/manageNetwork.cpp \
  common/dimsParams.cpp \
  common/selfNode.cpp 
  
# tracker core #
TRACKER_CORE_H = \
  tracker/accessFile.h \
  tracker/addressToCoins.h \
  tracker/bitcoinNodeMedium.h \
  tracker/blockInfoDatabase.h \
  tracker/clientRequestsManager.h \
  tracker/communicationProtocol.h \
  tracker/configureTrackerActionHandler.h \
  tracker/connectNodeAction.h \
  tracker/connectToTrackerRequest.h \
  tracker/databaseManager.h \
  tracker/getBalanceAction.h \
  tracker/getBalanceRequest.h \
  tracker/internalMediumProvider.h \
  tracker/internalOperationsMedium.h \
  tracker/originTransactionsDatabase.h \
  tracker/originAddressScaner.h \
  tracker/processNetwork.h \
  tracker/scanBitcoinNetworkRequest.h \
  tracker/segmentFileStorage.h \
  tracker/server.h \
  tracker/setResponseVisitor.h \
  tracker/simpleBuddy.h \
  tracker/supportTransactionsDatabase.h \
  tracker/synchronizationAction.h \
  tracker/synchronizationRequests.h \
  tracker/trackerController.h \
  tracker/trackerControllerStates.h \
  tracker/trackerControllerEvents.h \
  tracker/trackerNodeMedium.h \
  tracker/trackerNodesManager.h \
  tracker/transactionRecordManager.h \
  tracker/trackOriginAddressAction.h \
  tracker/validateTransactionsAction.h \
  tracker/validateTransactionActionEvents.h \
  tracker/validateTransactionsRequest.h \
  leveldbwrapper.h \
  txdb.h 
  
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
  tracker/bitcoinNodeMedium.cpp \
  tracker/blockInfoDatabase.cpp \
  tracker/connectNodeAction.cpp \
  tracker/connectToTrackerRequest.cpp \
  tracker/clientRequestsManager.cpp \
  tracker/getBalanceAction.cpp \
  tracker/getBalanceRequest.cpp \
  tracker/internalMediumProvider.cpp \
  tracker/internalOperationsMedium.cpp \
  tracker/originTransactionsDatabase.cpp \
  tracker/originAddressScaner.cpp \
  tracker/processNetwork.cpp \
  tracker/rpcTracker.cpp \
  tracker/scanBitcoinNetworkRequest.cpp \
  tracker/segmentFileStorage.cpp \
  tracker/server.cpp \
  tracker/setResponseVisitor.cpp \
  tracker/simpleBuddy.cpp \
  tracker/supportTransactionsDatabase.cpp \
  tracker/synchronizationAction.cpp \
  tracker/synchronizationRequests.cpp \
  tracker/trackerController.cpp \
  tracker/trackerControllerEvents.cpp \
  tracker/trackerControllerStates.cpp \
  tracker/trackerNodeMedium.cpp \
  tracker/trackerNodesManager.cpp \
  tracker/transactionRecordManager.cpp \
  tracker/trackOriginAddressAction.cpp \
  tracker/validateTransactionsAction.cpp \
  tracker/validateTransactionsRequest.cpp \
  leveldbwrapper.cpp \
  txdb.cpp
  
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
  main.cpp \
  net.cpp \
  noui.cpp \
  rpcmisc.cpp \
  rpcnet.cpp \
  rpcrawtransaction.cpp \
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

  libdims_tracker_a_SOURCES = \
  $(BITCOIN_CORE_H) \
  $(TRACKER_CORE_H) \
  $(TRACKER_CORE_CPP)
  
  
libbitcoin_cli_a_SOURCES = \
  rpcclient.cpp \
  $(BITCOIN_CORE_H)

nodist_libbitcoin_common_a_SOURCES = $(top_srcdir)/src/obj/build.h
#
# this repetition is temporary workaround
# dims_tracker binary #
dims_tracker_LDADD = \
  libdims_tracker.a \
  libbitcoin_server.a \
  libdims_tracker.a \
  libbitcoin_cli.a \
  libbitcoin_common.a \
  $(LIBLEVELDB) \
  $(LIBMEMENV)
dims_tracker_SOURCES = bitcoind.cpp
#

if TARGET_WINDOWS
dims_tracker_SOURCES += bitcoind-res.rc
endif

AM_CPPFLAGS += $(BDB_CPPFLAGS)
dims_tracker_LDADD += $(BOOST_LIBS) $(BDB_LIBS)

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
