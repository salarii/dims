// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "init.h"

#ifdef WIN32
#define MIN_CORE_FILEDESCRIPTORS 0
#else
#define MIN_CORE_FILEDESCRIPTORS 150
#endif

#if defined(HAVE_CONFIG_H)
#include "bitcoin-config.h"
#endif

#include "init.h"

#include "addrman.h"
#include "checkpoints.h"
#include "main.h"
#include "net.h"
#include "rpcserver.h"
#include "txdb.h"
#include "ui_interface.h"
#include "util.h"
#ifdef ENABLE_WALLET
#include "db.h"
#include "wallet.h"
#include "walletdb.h"
#endif

#include <inttypes.h>
#include <stdint.h>

#ifndef WIN32
#include <signal.h>
#endif

#include <boost/algorithm/string/predicate.hpp>
#include <boost/filesystem.hpp>
#include <boost/interprocess/sync/file_lock.hpp>
#include <openssl/crypto.h>

#include "common/actionHandler.h"
#include "common/manageNetwork.h"
#include "common/nodesManager.h"
#include "common/periodicActionExecutor.h"
#include "common/timeMedium.h"
#include "common/commandLine.h"
#include "common/originAddressScanner.h"

#include "monitor/processNetwork.h"
#include "monitor/monitorController.h"
#include "monitor/internalMediumProvider.h"
#include "monitor/server.h"
#include "monitor/clientRequestsManager.h"
#include "monitor/reputationTracer.h"
#include "monitor/noMediumHandling.h"
#include "monitor/registerRpcHooks.h"
#include "monitor/transactionRecordManager.h"

using namespace std;
using namespace boost;

namespace monitor
{


enum BindFlags {
	BF_NONE         = 0,
	BF_EXPLICIT     = (1U << 0),
	BF_REPORT_ERROR = (1U << 1)
};

volatile extern  bool fRequestShutdown;

void HandleSIGTERM(int)
{
	fRequestShutdown = true;
}

void HandleSIGHUP(int)
{
	fReopenDebugLog = true;
}

bool static InitError(const std::string &str)
{
	uiInterface.ThreadSafeMessageBox(str, "", CClientUIInterface::MSG_ERROR | CClientUIInterface::NOSHOWGUI);
	return false;
}

bool static InitWarning(const std::string &str)
{
	uiInterface.ThreadSafeMessageBox(str, "", CClientUIInterface::MSG_WARNING | CClientUIInterface::NOSHOWGUI);
	return true;
}

bool static Bind(const CService &addr, unsigned int flags) {
	if (!(flags & BF_EXPLICIT) && IsLimited(addr))
		return false;
	std::string strError;
	if (!BindListenPort(addr, strError)) {
		if (flags & BF_REPORT_ERROR)
			return InitError(strError);
		return false;
	}
	return true;
}

/** Initialize bitcoin.
 *  @pre Parameters should be parsed and config file should be read.
 */
bool AppInit(boost::thread_group& threadGroup)
{
	seed_insecure_rand();
	// ********************************************************* Step 1: setup
#ifdef _MSC_VER
	// Turn off Microsoft heap dump noise
	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_WARN, CreateFileA("NUL", GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, 0));
#endif
#if _MSC_VER >= 1400
	// Disable confusing "helpful" text message on abort, Ctrl-C
	_set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
#endif
#ifdef WIN32
	// Enable Data Execution Prevention (DEP)
	// Minimum supported OS versions: WinXP SP3, WinVista >= SP1, Win Server 2008
	// A failure is non-critical and needs no further attention!
#ifndef PROCESS_DEP_ENABLE
	// We define this here, because GCCs winbase.h limits this to _WIN32_WINNT >= 0x0601 (Windows 7),
	// which is not correct. Can be removed, when GCCs winbase.h is fixed!
#define PROCESS_DEP_ENABLE 0x00000001
#endif
	typedef BOOL (WINAPI *PSETPROCDEPPOL)(DWORD);
	PSETPROCDEPPOL setProcDEPPol = (PSETPROCDEPPOL)GetProcAddress(GetModuleHandleA("Kernel32.dll"), "SetProcessDEPPolicy");
	if (setProcDEPPol != NULL) setProcDEPPol(PROCESS_DEP_ENABLE);

	// Initialize Windows Sockets
	WSADATA wsadata;
	int ret = WSAStartup(MAKEWORD(2,2), &wsadata);
	if (ret != NO_ERROR || LOBYTE(wsadata.wVersion ) != 2 || HIBYTE(wsadata.wVersion) != 2)
	{
		return InitError(strprintf("Error: Winsock library failed to start (WSAStartup returned error %d)", ret));
	}
#endif
#ifndef WIN32
	umask(077);

	// Clean shutdown on SIGTERM
	struct sigaction sa;
	sa.sa_handler = HandleSIGTERM;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sigaction(SIGTERM, &sa, NULL);
	sigaction(SIGINT, &sa, NULL);

	// Reopen debug.log on SIGHUP
	struct sigaction sa_hup;
	sa_hup.sa_handler = HandleSIGHUP;
	sigemptyset(&sa_hup.sa_mask);
	sa_hup.sa_flags = 0;
	sigaction(SIGHUP, &sa_hup, NULL);

#if defined (__SVR4) && defined (__sun)
	// ignore SIGPIPE on Solaris
	signal(SIGPIPE, SIG_IGN);
#endif
#endif

	// ********************************************************* Step 2: parameter interactions

	if (mapArgs.count("-bind")) {
		// when specifying an explicit binding address, you want to listen on it
		// even when -connect or -proxy is specified
		if (SoftSetBoolArg("-listen", true))
			LogPrintf("AppInit2 : parameter interaction: -bind set -> setting -listen=1\n");
	}

	if (mapArgs.count("-connect") && mapMultiArgs["-connect"].size() > 0) {
		// when only connecting to trusted nodes, do not seed via DNS, or listen by default
		if (SoftSetBoolArg("-dnsseed", false))
			LogPrintf("AppInit2 : parameter interaction: -connect set -> setting -dnsseed=0\n");
		if (SoftSetBoolArg("-listen", false))
			LogPrintf("AppInit2 : parameter interaction: -connect set -> setting -listen=0\n");
	}

	if (mapArgs.count("-proxy")) {
		// to protect privacy, do not listen by default if a default proxy server is specified
		if (SoftSetBoolArg("-listen", false))
			LogPrintf("AppInit2 : parameter interaction: -proxy set -> setting -listen=0\n");
	}

	if (!GetBoolArg("-listen", true)) {
		// do not map ports or try to retrieve public IP when not listening (pointless)
		if (SoftSetBoolArg("-upnp", false))
			LogPrintf("AppInit2 : parameter interaction: -listen=0 -> setting -upnp=0\n");
		if (SoftSetBoolArg("-discover", false))
			LogPrintf("AppInit2 : parameter interaction: -listen=0 -> setting -discover=0\n");
	}



	// Make sure enough file descriptors are available
	int nBind = std::max((int)mapArgs.count("-bind"), 1);
	nMaxConnections = GetArg("-maxconnections", 128);
	nMaxConnections = std::max(std::min(nMaxConnections, (int)(FD_SETSIZE - nBind - MIN_CORE_FILEDESCRIPTORS)), 0);
	int nFD = RaiseFileDescriptorLimit(nMaxConnections + MIN_CORE_FILEDESCRIPTORS);
	if (nFD < MIN_CORE_FILEDESCRIPTORS)
		return InitError(_("Not enough file descriptors available."));
	if (nFD - MIN_CORE_FILEDESCRIPTORS < nMaxConnections)
		nMaxConnections = nFD - MIN_CORE_FILEDESCRIPTORS;

	// ********************************************************* Step 3: parameter-to-internal-flags

	fDebug = !mapMultiArgs["-debug"].empty();
	// Special-case: if -debug=0/-nodebug is set, turn off debugging messages
	const vector<string>& categories = mapMultiArgs["-debug"];
	if (GetBoolArg("-nodebug", false) || find(categories.begin(), categories.end(), string("0")) != categories.end())
		fDebug = false;

	// Check for -debugnet (deprecated)
	if (GetBoolArg("-debugnet", false))
		InitWarning(_("Warning: Deprecated argument -debugnet ignored, use -debug=net"));

	fBenchmark = GetBoolArg("-benchmark", false);
	mempool.setSanityCheck(GetBoolArg("-checkmempool", RegTest()));
	Checkpoints::fEnabled = GetBoolArg("-checkpoints", true);

	// -par=0 means autodetect, but nScriptCheckThreads==0 means no concurrency
	nScriptCheckThreads = GetArg("-par", 0);
	if (nScriptCheckThreads <= 0)
		nScriptCheckThreads += boost::thread::hardware_concurrency();
	if (nScriptCheckThreads <= 1)
		nScriptCheckThreads = 0;
	else if (nScriptCheckThreads > MAX_SCRIPTCHECK_THREADS)
		nScriptCheckThreads = MAX_SCRIPTCHECK_THREADS;

	fServer = GetBoolArg("-server", true);
	fPrintToConsole = GetBoolArg("-printtoconsole", false);
	fLogTimestamps = GetBoolArg("-logtimestamps", true);
#ifdef ENABLE_WALLET
	bool fDisableWallet = GetBoolArg("-disablewallet", false);
#endif

	if (mapArgs.count("-timeout"))
	{
		int nNewTimeout = GetArg("-timeout", 5000);
		if (nNewTimeout > 0 && nNewTimeout < 600000)
			nConnectTimeout = nNewTimeout;
	}

	// Continue to put "/P2SH/" in the coinbase to monitor
	// BIP16 support.
	// This can be removed eventually...
	const char* pszP2SH = "/P2SH/";
	COINBASE_FLAGS << std::vector<unsigned char>(pszP2SH, pszP2SH+strlen(pszP2SH));

#ifdef ENABLE_WALLET
	if (mapArgs.count("-paytxfee"))
	{
		if (!ParseMoney(mapArgs["-paytxfee"], nTransactionFee))
			return InitError(strprintf(_("Invalid amount for -paytxfee=<amount>: '%s'"), mapArgs["-paytxfee"]));
		if (nTransactionFee > 0.25 * COIN)
			InitWarning(_("Warning: -paytxfee is set very high! This is the transaction fee you will pay if you send a transaction."));
	}
	bSpendZeroConfChange = GetArg("-spendzeroconfchange", true);

	strWalletFile = GetArg("-wallet", "wallet.dat");
#endif
	// ********************************************************* Step 4: application initialization: dir lock, daemonize, pidfile, debug log

	std::string strDataDir = GetDataDir( common::AppType::Monitor ).string();
#ifdef ENABLE_WALLET
	// Wallet file must be a plain filename without a directory
	if (strWalletFile != boost::filesystem::basename(strWalletFile) + boost::filesystem::extension(strWalletFile))
		return InitError(strprintf(_("Wallet %s resides outside data directory %s"), strWalletFile, strDataDir));
#endif
	boost::filesystem::path pathLockFile = GetDataDir(common::AppType::Monitor) / ".lock";
	FILE* file = fopen(pathLockFile.string().c_str(), "a"); // empty lock file; created if it doesn't exist.
	if (file) fclose(file);
	static boost::interprocess::file_lock lock(pathLockFile.string().c_str());
	if (!lock.try_lock())
		return InitError(strprintf(_("Cannot obtain a lock on data directory %s. Bitcoin is probably already running."), strDataDir));

	if (GetBoolArg("-shrinkdebugfile", !fDebug))
		ShrinkDebugFile();
	LogPrintf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
	LogPrintf("Bitcoin version %s (%s)\n", FormatFullVersion(), CLIENT_DATE);
	LogPrintf("Using OpenSSL version %s\n", SSLeay_version(SSLEAY_VERSION));
	if (!fLogTimestamps)
		LogPrintf("Startup time: %s\n", DateTimeStrFormat("%Y-%m-%d %H:%M:%S", GetTime()));
	LogPrintf("Default data directory %s\n", GetDefaultDataDir(common::AppType::Monitor).string());
	LogPrintf("Using data directory %s\n", strDataDir);
	LogPrintf("Using at most %i connections (%i file descriptors available)\n", nMaxConnections, nFD);
	std::ostringstream strErrors;

	int64_t nStart;

	// ********************************************************* Step 5: verify wallet database integrity
#ifdef ENABLE_WALLET
	if (!fDisableWallet) {
		uiInterface.InitMessage(_("Verifying wallet..."));

		if (!bitdb.Open(GetDataDir(common::AppType::Monitor)))
		{
			// try moving the database env out of the way
			boost::filesystem::path pathDatabase = GetDataDir(common::AppType::Monitor) / "database";
			boost::filesystem::path pathDatabaseBak = GetDataDir(common::AppType::Monitor) / strprintf("database.%"PRId64".bak", GetTime());
			try {
				boost::filesystem::rename(pathDatabase, pathDatabaseBak);
				LogPrintf("Moved old %s to %s. Retrying.\n", pathDatabase.string(), pathDatabaseBak.string());
			} catch(boost::filesystem::filesystem_error &error) {
				 // failure is ok (well, not really, but it's not worse than what we started with)
			}

			// try again
			if (!bitdb.Open(GetDataDir(common::AppType::Monitor))) {
				// if it still fails, it probably means we can't even create the database env
				string msg = strprintf(_("Error initializing wallet database environment %s!"), strDataDir);
				return InitError(msg);
			}
		}

		if (GetBoolArg("-salvagewallet", false))
		{
			// Recover readable keypairs:
			if (!CWalletDB::Recover(bitdb, strWalletFile, true))
				return false;
		}

		if (filesystem::exists(GetDataDir(common::AppType::Monitor) / strWalletFile))
		{
			CDBEnv::VerifyResult r = bitdb.Verify(strWalletFile, CWalletDB::Recover);
			if (r == CDBEnv::RECOVER_OK)
			{
				string msg = strprintf(_("Warning: wallet.dat corrupt, data salvaged!"
										 " Original wallet.dat saved as wallet.{timestamp}.bak in %s; if"
										 " your balance or transactions are incorrect you should"
										 " restore from a backup."), strDataDir);
				InitWarning(msg);
			}
			if (r == CDBEnv::RECOVER_FAIL)
				return InitError(_("wallet.dat corrupt, salvage failed"));
		}
	} // (!fDisableWallet)
#endif // ENABLE_WALLET
	// ********************************************************* Step 6: network initialization

	RegisterNodeSignals(GetNodeSignals());

	int nSocksVersion = GetArg("-socks", 5);
	if (nSocksVersion != 4 && nSocksVersion != 5)
		return InitError(strprintf(_("Unknown -socks proxy version requested: %i"), nSocksVersion));

	if (mapArgs.count("-onlynet")) {
		std::set<enum Network> nets;
		BOOST_FOREACH(std::string snet, mapMultiArgs["-onlynet"]) {
			enum Network net = ParseNetwork(snet);
			if (net == NET_UNROUTABLE)
				return InitError(strprintf(_("Unknown network specified in -onlynet: '%s'"), snet));
			nets.insert(net);
		}
		for (int n = 0; n < NET_MAX; n++) {
			enum Network net = (enum Network)n;
			if (!nets.count(net))
				SetLimited(net);
		}
	}
#if defined(USE_IPV6)
#if ! USE_IPV6
	else
		SetLimited(NET_IPV6);
#endif
#endif

	CService addrProxy;
	bool fProxy = false;
	if (mapArgs.count("-proxy")) {
		addrProxy = CService(mapArgs["-proxy"], 9050);
		if (!addrProxy.IsValid())
			return InitError(strprintf(_("Invalid -proxy address: '%s'"), mapArgs["-proxy"]));

		if (!IsLimited(NET_IPV4))
			SetProxy(NET_IPV4, addrProxy, nSocksVersion);
		if (nSocksVersion > 4) {
#ifdef USE_IPV6
			if (!IsLimited(NET_IPV6))
				SetProxy(NET_IPV6, addrProxy, nSocksVersion);
#endif
			SetNameProxy(addrProxy, nSocksVersion);
		}
		fProxy = true;
	}
	// -onion can override normal proxy, -noonion disables tor entirely
	// -tor here is a temporary backwards compatibility measure
	if (mapArgs.count("-tor"))
		printf("Notice: option -tor has been replaced with -onion and will be removed in a later version.\n");
	if (!(mapArgs.count("-onion") && mapArgs["-onion"] == "0") &&
		!(mapArgs.count("-tor") && mapArgs["-tor"] == "0") &&
		 (fProxy || mapArgs.count("-onion") || mapArgs.count("-tor"))) {
		CService addrOnion;
		if (!mapArgs.count("-onion") && !mapArgs.count("-tor"))
			addrOnion = addrProxy;
		else
			addrOnion = mapArgs.count("-onion")?CService(mapArgs["-onion"], 9050):CService(mapArgs["-tor"], 9050);
		if (!addrOnion.IsValid())
			return InitError(strprintf(_("Invalid -onion address: '%s'"), mapArgs.count("-onion")?mapArgs["-onion"]:mapArgs["-tor"]));
		SetProxy(NET_TOR, addrOnion, 5);
		SetReachable(NET_TOR);
	}

	// see Step 2: parameter interactions for more information about these
	fNoListen = !GetBoolArg("-listen", true);
	fDiscover = GetBoolArg("-discover", true);
	fNameLookup = GetBoolArg("-dns", true);

	bool fBound = false;
	if (!fNoListen) {
		if (mapArgs.count("-bind")) {
			BOOST_FOREACH(std::string strBind, mapMultiArgs["-bind"]) {
				CService addrBind;
				if (!Lookup(strBind.c_str(), addrBind, GetListenPort<CChainParams>(), false))
					return InitError(strprintf(_("Cannot resolve -bind address: '%s'"), strBind));
				fBound |= Bind(addrBind, (BF_EXPLICIT | BF_REPORT_ERROR));
			}
		}
		else {
			struct in_addr inaddr_any;
			inaddr_any.s_addr = INADDR_ANY;
#ifdef USE_IPV6
			fBound |= Bind(CService(in6addr_any, GetListenPort<CChainParams>()), BF_NONE);
#endif
			fBound |= Bind(CService(inaddr_any, GetListenPort<CChainParams>()), !fBound ? BF_REPORT_ERROR : BF_NONE);
		}
		if (!fBound)
			return InitError(_("Failed to listen on any port. Use -listen=0 if you want this."));
	}

	if (mapArgs.count("-externalip")) {
		BOOST_FOREACH(string strAddr, mapMultiArgs["-externalip"]) {
			CService addrLocal(strAddr, GetListenPort<CChainParams>(), fNameLookup);
			if (!addrLocal.IsValid())
				return InitError(strprintf(_("Cannot resolve -externalip address: '%s'"), strAddr));
			AddLocal(CService(strAddr, GetListenPort<CChainParams>(), fNameLookup), LOCAL_MANUAL);
		}
	}

	BOOST_FOREACH(string strDest, mapMultiArgs["-seednode"])
		AddOneShot(strDest);

	// ********************************************************* Step 7: load block chain

	fReindex = GetBoolArg("-reindex", false);

	// Upgrading to 0.8; hard-link the old blknnnn.dat files into /blocks/
	filesystem::path blocksDir = GetDataDir(common::AppType::Monitor) / "blocks";
	if (!filesystem::exists(blocksDir))
	{
		filesystem::create_directories(blocksDir);
		bool linked = false;
		for (unsigned int i = 1; i < 10000; i++) {
			filesystem::path source = GetDataDir(common::AppType::Monitor) / strprintf("blk%04u.dat", i);
			if (!filesystem::exists(source)) break;
			filesystem::path dest = blocksDir / strprintf("blk%05u.dat", i-1);
			try {
				filesystem::create_hard_link(source, dest);
				LogPrintf("Hardlinked %s -> %s\n", source.string(), dest.string());
				linked = true;
			} catch (filesystem::filesystem_error & e) {
				// Note: hardlink creation failing is not a disaster, it just means
				// blocks will get re-downloaded from peers.
				LogPrintf("Error hardlinking blk%04u.dat : %s\n", i, e.what());
				break;
			}
		}
		if (linked)
		{
			fReindex = true;
		}
	}

	bool fLoaded = false;
	while (!fLoaded) {
		bool fReset = fReindex;
		std::string strLoadError;

		uiInterface.InitMessage(_("Loading block index..."));

		nStart = GetTimeMillis();
		do {
			try {
				UnloadBlockIndex();
/*
				 if (!LoadBlockIndex()) {
					strLoadError = _("Error loading block database");
					break;
				}
*/
				// If the loaded chain has a wrong genesis, bail out immediately
				// (we're likely using a testnet datadir, or the other way around).
				if (!mapBlockIndex.empty() && chainActive.Genesis() == NULL)
					return InitError(_("Incorrect or no genesis block found. Wrong datadir for network?"));

				// Initialize the block index (no-op if non-empty database was already loaded)
				if (!InitBlockIndex()) {
					strLoadError = _("Error initializing block database");
					break;
				}

				// Check for changed -txindex state
				if (fTxIndex != GetBoolArg("-txindex", false)) {
					strLoadError = _("You need to rebuild the database using -reindex to change -txindex");
					break;
				}

				uiInterface.InitMessage(_("Verifying blocks..."));
				if (!VerifyDB(GetArg("-checklevel", 3),
							  GetArg("-checkblocks", 288))) {
					strLoadError = _("Corrupted block database detected");
					break;
				}
			} catch(std::exception &e) {
				if (fDebug) LogPrintf("%s\n", e.what());
				strLoadError = _("Error opening block database");
				break;
			}

			fLoaded = true;
		} while(false);

		if (!fLoaded) {
			// first suggest a reindex
			if (!fReset) {
				bool fRet = uiInterface.ThreadSafeMessageBox(
					strLoadError + ".\n\n" + _("Do you want to rebuild the block database now?"),
					"", CClientUIInterface::MSG_ERROR | CClientUIInterface::BTN_ABORT);
				if (fRet) {
					fReindex = true;
					fRequestShutdown = false;
				} else {
					LogPrintf("Aborted block database rebuild. Exiting.\n");
					return false;
				}
			} else {
				return InitError(strLoadError);
			}
		}
	}

	// As LoadBlockIndex can take several minutes, it's possible the user
	// requested to kill the GUI during the last operation. If so, exit.
	// As the program has not fully started yet, Shutdown() is possibly overkill.
	if (fRequestShutdown)
	{
		LogPrintf("Shutdown requested. Exiting.\n");
		return false;
	}
	// ********************************************************* Step 8: load wallet
#ifdef ENABLE_WALLET
	if (fDisableWallet) {
		pwalletMain = NULL;
		LogPrintf("Wallet disabled!\n");
	} else {
		if (GetBoolArg("-zapwallettxes", false)) {
			uiInterface.InitMessage(_("Zapping all transactions from wallet..."));

			pwalletMain = CWallet::getInstance(strWalletFile);
			DBErrors nZapWalletRet = pwalletMain->ZapWalletTx();
			if (nZapWalletRet != DB_LOAD_OK) {
				uiInterface.InitMessage(_("Error loading wallet.dat: Wallet corrupted"));
				return false;
			}

			delete pwalletMain;
			pwalletMain = NULL;
		}

		uiInterface.InitMessage(_("Loading wallet..."));

		nStart = GetTimeMillis();
		bool fFirstRun = true;
		pwalletMain = CWallet::getInstance(strWalletFile);
		DBErrors nLoadWalletRet = pwalletMain->LoadWallet(fFirstRun);
		if (nLoadWalletRet != DB_LOAD_OK)
		{
			if (nLoadWalletRet == DB_CORRUPT)
				strErrors << _("Error loading wallet.dat: Wallet corrupted") << "\n";
			else if (nLoadWalletRet == DB_NONCRITICAL_ERROR)
			{
				string msg(_("Warning: error reading wallet.dat! All keys read correctly, but transaction data"
							 " or address book entries might be missing or incorrect."));
				InitWarning(msg);
			}
			else if (nLoadWalletRet == DB_TOO_NEW)
				strErrors << _("Error loading wallet.dat: Wallet requires newer version of Bitcoin") << "\n";
			else if (nLoadWalletRet == DB_NEED_REWRITE)
			{
				strErrors << _("Wallet needed to be rewritten: restart Bitcoin to complete") << "\n";
				LogPrintf("%s", strErrors.str());
				return InitError(strErrors.str());
			}
			else
				strErrors << _("Error loading wallet.dat") << "\n";
		}

		if (GetBoolArg("-upgradewallet", fFirstRun))
		{
			int nMaxVersion = GetArg("-upgradewallet", 0);
			if (nMaxVersion == 0) // the -upgradewallet without argument case
			{
				LogPrintf("Performing wallet upgrade to %i\n", FEATURE_LATEST);
				nMaxVersion = CLIENT_VERSION;
				pwalletMain->SetMinVersion(FEATURE_LATEST); // permanently upgrade the wallet immediately
			}
			else
				LogPrintf("Allowing wallet upgrade up to %i\n", nMaxVersion);
			if (nMaxVersion < pwalletMain->GetVersion())
				strErrors << _("Cannot downgrade wallet") << "\n";
			pwalletMain->SetMaxVersion(nMaxVersion);
		}

		if (fFirstRun)
		{
			// Create new keyUser and set as default key
			RandAddSeedPerfmon();

			CPubKey newDefaultKey;
			if (pwalletMain->GetKeyFromPool(newDefaultKey)) {
				pwalletMain->SetDefaultKey(newDefaultKey);
				if (!pwalletMain->SetAddressBook(pwalletMain->vchDefaultKey.GetID(), "", "receive"))
					strErrors << _("Cannot write default address") << "\n";
			}

			pwalletMain->SetBestChain(chainActive.GetLocator());
		}

		LogPrintf("%s", strErrors.str());
		LogPrintf(" wallet      %15"PRId64"ms\n", GetTimeMillis() - nStart);

		RegisterWallet(pwalletMain);

	} // (!fDisableWallet)
#else // ENABLE_WALLET
	LogPrintf("No wallet compiled in!\n");
#endif // !ENABLE_WALLET
	// ********************************************************* Step 9: import blocks

	common::COriginAddressScanner::getInstance()->setStorage( monitor::CTransactionRecordManager::getInstance() );
/* create  threads of  action  handler */
	threadGroup.create_thread( boost::bind( &common::COriginAddressScanner::loop, common::COriginAddressScanner::getInstance() ) );

	threadGroup.create_thread( boost::bind( &common::CSegmentFileStorage::flushLoop, common::CSegmentFileStorage::getInstance() ) );

	threadGroup.create_thread( boost::bind( &common::CActionHandler< common::CMonitorTypes >::loop, common::CActionHandler< common::CMonitorTypes >::getInstance() ) );

	threadGroup.create_thread( boost::bind( &common::CTimeMedium< common::CMonitorBaseMedium >::workLoop, common::CTimeMedium< common::CMonitorBaseMedium >::getInstance() ) );

	threadGroup.create_thread( boost::bind( &monitor::CClientRequestsManager::processRequestLoop, monitor::CClientRequestsManager::getInstance() ) );

	threadGroup.create_thread( boost::bind( &common::CCommandLine::workLoop, common::CCommandLine::getInstance() ) );

	common::CActionHandler< common::CMonitorTypes >::getInstance()->addConnectionProvider( (common::CConnectionProvider< common::CMonitorTypes >*)monitor::CInternalMediumProvider::getInstance() );

	common::CActionHandler< common::CMonitorTypes >::getInstance()->addConnectionProvider( (common::CConnectionProvider< common::CMonitorTypes >*)monitor::CReputationTracker::getInstance() );

		common::CActionHandler< common::CMonitorTypes >::getInstance()->addConnectionProvider( (common::CConnectionProvider< common::CMonitorTypes >*)CErrorMediumProvider::getInstance() );

	common::CManageNetwork::getInstance()->registerNodeSignals( CProcessNetwork::getInstance() );

	common::CManageNetwork::getInstance()->connectToNetwork( threadGroup );

	common::CPeriodicActionExecutor< common::CMonitorTypes > * periodicActionExecutor
			= common::CPeriodicActionExecutor< common::CMonitorTypes >::getInstance();

	threadGroup.create_thread(boost::bind(&common::CPeriodicActionExecutor< common::CMonitorTypes >::processingLoop, periodicActionExecutor ));

	// ********************************************************* Step 10: load peers

	CMonitorController::getInstance();


	nStart = GetTimeMillis();

	{
		CAddrDB adb;
		if (!adb.Read(addrman))
			LogPrintf("Invalid or missing peers.dat; recreating\n");
	}

	LogPrintf("Loaded %i addresses from peers.dat  %"PRId64"ms\n",
		   addrman.size(), GetTimeMillis() - nStart);

	// ********************************************************* Step 11: start node

	if (!CheckDiskSpace())
		return false;

	if (!strErrors.str().empty())
		return InitError(strErrors.str());

	RandAddSeedPerfmon();

	//// debug print
	LogPrintf("mapBlockIndex.size() = %"PRIszu"\n",   mapBlockIndex.size());
	LogPrintf("nBestHeight = %d\n",                   chainActive.Height());
#ifdef ENABLE_WALLET
	LogPrintf("setKeyPool.size() = %"PRIszu"\n",      pwalletMain ? pwalletMain->setKeyPool.size() : 0);
	LogPrintf("mapWallet.size() = %"PRIszu"\n",       pwalletMain ? pwalletMain->mapWallet.size() : 0);
	LogPrintf("mapAddressBook.size() = %"PRIszu"\n",  pwalletMain ? pwalletMain->mapAddressBook.size() : 0);
#endif
	// InitRPCMining is needed here so getwork/getblocktemplate in the GUI debug console works properly.
  //  InitRPCMining();

	m_setTransaction.connect( boost::bind( &monitor::CInternalMediumProvider::setTransaction, monitor::CInternalMediumProvider::getInstance(), _1, _2 ) );
	m_setMerkleBlock.connect( boost::bind( &monitor::CInternalMediumProvider::setMerkleBlock, monitor::CInternalMediumProvider::getInstance(), _1, _2 ) );

	if (fServer)
		StartRPCThreads();

	StartNode(threadGroup);

//	StopHook.connect( &StartShutdown );
	monitor::registerHooks();

	monitor::runServer();

	// ********************************************************* Step 12: finished

	uiInterface.InitMessage(_("Done loading"));

#ifdef ENABLE_WALLET
	if (pwalletMain) {

		// Run a thread to flush wallet periodically
		threadGroup.create_thread(boost::bind(&ThreadFlushWalletDB, boost::ref(pwalletMain->strWalletFile)));
	}
#endif

	return !fRequestShutdown;
}

void Shutdown()
{
	LogPrintf("Shutdown : In progress...\n");
	static CCriticalSection cs_Shutdown;
	TRY_LOCK(cs_Shutdown, lockShutdown);
	if (!lockShutdown) return;

	RenameThread("bitcoin-shutoff");
#ifdef ENABLE_WALLET
	if (pwalletMain)
		bitdb.Flush(false);
#endif
	StopNode();
	UnregisterNodeSignals(GetNodeSignals());
	{
		LOCK(cs_main);
#ifdef ENABLE_WALLET
		if (pwalletMain)
			pwalletMain->SetBestChain(chainActive.GetLocator());
#endif

	}
#ifdef ENABLE_WALLET
	if (pwalletMain)
		bitdb.Flush(true);
#endif
	boost::filesystem::remove(GetPidFile());
	UnregisterAllWallets();
#ifdef ENABLE_WALLET
	if (pwalletMain)
		delete pwalletMain;
#endif
	LogPrintf("Shutdown : done\n");
}

}
