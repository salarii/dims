// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

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

#include "client/settingsConnectionProvider.h"

#include "common/actionHandler.h"

#include "clientControl.h"

using namespace std;
using namespace boost;

namespace client
{

enum BindFlags {
	BF_NONE         = 0,
	BF_EXPLICIT     = (1U << 0),
	BF_REPORT_ERROR = (1U << 1)
};

volatile bool fRequestShutdown = false;

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
bool AppInit1(boost::thread_group& threadGroup)
{
	seed_insecure_rand();
	common::CDimsParams::setAppType( common::AppType::Client);
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

	// ********************************************************* Step 3: parameter-to-internal-flags
	if (mapArgs.count("-tracker"))
	{
		client::CSettingsConnectionProvider::getInstance()->setTrackerIp( GetArg( "-tracker", "127.0.0.1" ) );
	}
	// -par=0 means autodetect, but nScriptCheckThreads==0 means no server
	if (mapArgs.count("-timeout"))
	{
		int nNewTimeout = GetArg("-timeout", 5000);
		if (nNewTimeout > 0 && nNewTimeout < 600000)
			nConnectTimeout = nNewTimeout;
	}
#ifdef ENABLE_WALLET
	bool fDisableWallet = GetBoolArg("-disablewallet", false);
#endif

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

	std::string strDataDir = GetDataDir(common::AppType::Client).string();
#ifdef ENABLE_WALLET
	// Wallet file must be a plain filename without a directory
	if (strWalletFile != boost::filesystem::basename(strWalletFile) + boost::filesystem::extension(strWalletFile))
		return InitError(strprintf(_("Wallet %s resides outside data directory %s"), strWalletFile, strDataDir));
#endif
	// Make sure only a single Bitcoin process is using the data directory.
	boost::filesystem::path pathLockFile = GetDataDir(common::AppType::Client) / ".lock";
	FILE* file = fopen(pathLockFile.string().c_str(), "a"); // empty lock file; created if it doesn't exist.
	if (file) fclose(file);
	static boost::interprocess::file_lock lock(pathLockFile.string().c_str());
	if (!lock.try_lock())
		return InitError(strprintf(_("Cannot obtain a lock on data directory %s. Ratcoin is probably already running."), strDataDir));

	if (GetBoolArg("-shrinkdebugfile", !fDebug))
		ShrinkDebugFile();
	LogPrintf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
	LogPrintf("Ratcoin version %s (%s)\n", FormatFullVersion(), CLIENT_DATE);
	LogPrintf("Using OpenSSL version %s\n", SSLeay_version(SSLEAY_VERSION));
	if (!fLogTimestamps)
		LogPrintf("Startup time: %s\n", DateTimeStrFormat("%Y-%m-%d %H:%M:%S", GetTime()));
	LogPrintf("Default data directory %s\n", GetDefaultDataDir(common::AppType::Client).string());
	LogPrintf("Using data directory %s\n", strDataDir);
	std::ostringstream strErrors;

	if (nScriptCheckThreads) {
		LogPrintf("Using %u threads for script verification\n", nScriptCheckThreads);
		for (int i=0; i<nScriptCheckThreads-1; i++)
			threadGroup.create_thread(&ThreadScriptCheck);
	}

	int64_t nStart;

	// ********************************************************* Step 5: verify wallet database integrity
#ifdef ENABLE_WALLET
	if (!fDisableWallet) {
		uiInterface.InitMessage(_("Verifying wallet..."));

		if (!bitdb.Open(GetDataDir(common::AppType::Client)))
		{
			// try moving the database env out of the way
			boost::filesystem::path pathDatabase = GetDataDir(common::AppType::Client) / "database";
			boost::filesystem::path pathDatabaseBak = GetDataDir(common::AppType::Client) / strprintf("database.%"PRId64".bak", GetTime());
			try {
				boost::filesystem::rename(pathDatabase, pathDatabaseBak);
				LogPrintf("Moved old %s to %s. Retrying.\n", pathDatabase.string(), pathDatabaseBak.string());
			} catch(boost::filesystem::filesystem_error &error) {
				 // failure is ok (well, not really, but it's not worse than what we started with)
			}

			// try again
			if (!bitdb.Open(GetDataDir(common::AppType::Client))) {
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

		if (filesystem::exists(GetDataDir(common::AppType::Client) / strWalletFile))
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
	// ********************************************************* Step 6:

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
/*
			CPubKey newDefaultKey;
			if (pwalletMain->GetKeyFromPool(newDefaultKey)) {
				pwalletMain->SetDefaultKey(newDefaultKey);
				if (!pwalletMain->SetAddressBook(pwalletMain->vchDefaultKey.GetID(), "", "receive"))
					strErrors << _("Cannot write default address") << "\n";
			}*/
		}

		LogPrintf("%s", strErrors.str());
		LogPrintf(" wallet      %15"PRId64"ms\n", GetTimeMillis() - nStart);

		RegisterWallet(pwalletMain);

	} // (!fDisableWallet)
#else // ENABLE_WALLET
	LogPrintf("No wallet compiled in!\n");
#endif // !ENABLE_WALLET
	// ********************************************************* Step 9: import blocks

	  // ********************************************************* Step 10: load peers

 /*   uiInterface.InitMessage(_("Loading addresses..."));

	nStart = GetTimeMillis();

	{
		CAddrDB adb;
		if (!adb.Read(addrman))
			LogPrintf("Invalid or missing peers.dat; recreating\n");
	}

	LogPrintf("Loaded %i addresses from peers.dat  %"PRId64"ms\n",
		   addrman.size(), GetTimeMillis() - nStart);*/

	// ********************************************************* Step 11: start node

#ifdef ENABLE_WALLET
	LogPrintf("setKeyPool.size() = %"PRIszu"\n",      pwalletMain ? pwalletMain->setKeyPool.size() : 0);
	LogPrintf("mapWallet.size() = %"PRIszu"\n",       pwalletMain ? pwalletMain->mapWallet.size() : 0);
	LogPrintf("mapAddressBook.size() = %"PRIszu"\n",  pwalletMain ? pwalletMain->mapAddressBook.size() : 0);
#endif

	//StartNode(threadGroup);
	// InitRPCMining is needed here so getwork/getblocktemplate in the GUI debug console works properly.


	// ********************************************************* Step 12: finished

	uiInterface.InitMessage(_("Done loading"));
	CClientControl::getInstance();
#ifdef ENABLE_WALLET
	if (pwalletMain) {
		// Add wallet transactions that aren't already in a block to mapTransactions
		pwalletMain->ReacceptWalletTransactions();

		// Run a thread to flush wallet periodically
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
