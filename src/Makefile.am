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


SUBDIRS = . $(BUILD_QT) $(BUILD_TEST) monitor seed appLib
DIST_SUBDIRS = . qt test
.PHONY: FORCE
	
COMMON_H = \
  common/accessFile.h \
  common/actionHandler.h \
  common/action.h \
  common/analyseTransaction.h \
  common/authenticationProvider.h \
  common/bitcoinNodeMedium.h \
  common/blockInfoDatabase.h \
  common/commandLine.h \
  common/communicationProtocol.h \
  common/commonEvents.h \
  common/responses.h \
  common/requests.h \
  common/databaseManager.h \
  common/dimsParams.h \
  common/filters.h \
  common/identifyRequest.h \
  common/manageNetwork.h \
  common/networkActionRegister.h \
  common/nodeMedium.h \
  common/nodeMessages.h \
  common/nodesManager.h \
  common/noMediumHandling.h \
  common/originAddressScanner.h \
  common/originTransactionsDatabase.h \
  common/periodicActionExecutor.h \
  common/pingAction.h \
  common/responseVisitorInternal.h \
  common/requestHandler.h \
  common/request.h \
  common/scheduleAbleAction.h \
  common/scheduledActionManager.h \
  common/segmentFileStorage.h \
  common/selfNode.h \
  common/simpleBuddy.h \
  common/commonStruct.h \
  common/supportTransactionsDatabase.h \
  common/timeMedium.h \
  common/transactionStatus.h \
  common/visitorConfigurationUtilities.h 
  
COMMON_CPP = \
  common/accessFile.cpp \
  common/action.cpp \
  common/actionHandler.cpp \
  common/analyseTransaction.cpp \
  common/authenticationProvider.cpp \
  common/bitcoinNodeMedium.cpp \
  common/blockInfoDatabase.cpp \
  common/commandLine.cpp \
  common/communicationProtocol.cpp \
  common/responses.cpp \
  common/requests.cpp	 \	
  common/databaseManager.cpp \
  common/dimsParams.cpp \
  common/manageNetwork.cpp \
  common/networkActionRegister.cpp \
  common/nodeMedium.cpp \
  common/nodesManager.cpp \
  common/noMediumHandling.cpp \
  common/originAddressScanner.cpp \
  common/originTransactionsDatabase.cpp \
  common/periodicActionExecutor.cpp \
  common/requestHandler.cpp \
  common/request.cpp \
  common/scheduledActionManager.cpp \
  common/segmentFileStorage.cpp \
  common/selfNode.cpp \
  common/segmentFileStorage.cpp \
  common/simpleBuddy.cpp \
  common/supportTransactionsDatabase.cpp \
  common/timeMedium.cpp
  
# tracker core #
TRACKER_CORE_H = \
  tracker/addressToCoins.h \
  tracker/clientRequestsManager.h \
  tracker/communicationProtocol.h \
  tracker/configureTrackerActionHandler.h \
  tracker/connectNetworkAction.h \
  tracker/connectNodeAction.h \
  tracker/controller.h \
  tracker/databaseManager.h \
  tracker/getBalanceAction.h \	
  tracker/init.h \
  tracker/internalMediumProvider.h \
  tracker/internalOperationsMedium.h \
  tracker/passTransactionAction.h \
  tracker/pingAction.h \
  tracker/processNetwork.h \
  tracker/provideInfoAction.h \
  tracker/recognizeNetworkAction.h \
  tracker/registerAction.h \
  tracker/registerRpcHooks.h \
  tracker/requests.h \
  tracker/server.h \
  tracker/setResponseVisitor.h \
  tracker/synchronizationAction.h \
  tracker/controllerEvents.h \
  tracker/trackerNodeMedium.h \
  tracker/trackerNodesManager.h \
  tracker/transactionRecordManager.h \
  tracker/trackOriginAddressAction.h \
  tracker/updateNetworkDataAction.h \
  tracker/validateTransactionsAction.h \
  tracker/validateTransactionActionEvents.h
  
  
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
  wallet.h \
  leveldbwrapper.h \
  txdb.h 

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
  tracker/addressToCoins.cpp \
  tracker/connectNetworkAction.cpp \
  tracker/connectNodeAction.cpp \
  tracker/controller.cpp \
  tracker/controllerEvents.cpp \
  tracker/clientRequestsManager.cpp \
  tracker/getBalanceAction.cpp \
  tracker/init.cpp \
  tracker/internalMediumProvider.cpp \
  tracker/internalOperationsMedium.cpp \
  tracker/passTransactionAction.cpp \
  tracker/pingAction.cpp \
  tracker/processNetwork.cpp \
  tracker/provideInfoAction.cpp \
  tracker/recognizeNetworkAction.cpp \
  tracker/registerAction.cpp \
  tracker/registerRpcHooks.cpp \
  tracker/requests.cpp \
  tracker/rpcTracker.cpp \
  tracker/server.cpp \
  tracker/setResponseVisitor.cpp \
  tracker/synchronizationAction.cpp \
  tracker/trackerNodeMedium.cpp \
  tracker/trackerNodesManager.cpp \
  tracker/transactionRecordManager.cpp \
  tracker/trackOriginAddressAction.cpp \
  tracker/updateNetworkDataAction.cpp \
  tracker/validateTransactionsAction.cpp

  
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
  leveldbwrapper.cpp \
  txdb.cpp \
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
dims_tracker_SOURCES = tracker/tracker.cpp
#

if TARGET_WINDOWS
dims_tracker_SOURCES += bitcoind-res.rc
endif

AM_CPPFLAGS += $(BDB_CPPFLAGS)
dims_tracker_LDADD += $(BOOST_LIBS) $(BDB_LIBS)

#



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
