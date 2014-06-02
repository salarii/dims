#if defined(HAVE_CONFIG_H)
#include "bitcoin-config.h"
#endif

#include "main.h"
#include "rpcserver.h"
#include "ui_interface.h"
#include "wallet.h"

#include <stdint.h>

#include <boost/filesystem/operations.hpp>
#include <boost/bind.hpp>

#include "monitor/init.h"
#include "util.h"
#include "chainparams.h"
#include "rpcclient.h"

#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

namespace monitor
{

volatile bool fRequestShutdown = false;

static bool fDaemon;

void StartShutdown()
{
	fRequestShutdown = true;
}
bool ShutdownRequested()
{
	return fRequestShutdown;
}

void DetectShutdownThread(boost::thread_group* threadGroup)
{
	bool fShutdown = ShutdownRequested();
	// Tell the main threads to shutdown.
	while (!fShutdown)
	{
		MilliSleep(200);
		fShutdown = ShutdownRequested();
	}
	if (threadGroup)
	{
		threadGroup->interrupt_all();
		threadGroup->join_all();
	}
}

bool AppInit(int argc, char* argv[])
{
	boost::thread_group threadGroup;
	boost::thread* detectShutdownThread = NULL;

	bool fRet = false;
	try
	{
		//
		// Parameters
		//
		// If Qt is used, parameters/tracker.conf are parsed in qt/bitcoin.cpp's main()
		ParseParameters(argc, argv);
		if (!boost::filesystem::is_directory(GetDataDir(common::AppType::Tracker, false)))
		{
			fprintf(stderr, "Error: Specified data directory \"%s\" does not exist.\n", mapArgs["-datadir"].c_str());
			return false;
		}
		ReadConfigFile(mapArgs, mapMultiArgs);
		// Check for -testnet or -regtest parameter (TestNet() calls are only valid after this clause)
		if (!SelectParamsFromCommandLine()|| !common::SelectRatcoinParamsFromCommandLine() ) {
			fprintf(stderr, "Error: Invalid combination of -regtest and -testnet.\n");
			return false;
		}

		if (mapArgs.count("-?") || mapArgs.count("--help"))
		{
			// First part of help message is specific to bitcoind / RPC client
			std::string strUsage = _("Bitcoin Core Daemon") + " " + _("version") + " " + FormatFullVersion() + "\n\n" +
				_("Usage:") + "\n" +
				  "  bitcoind [options]                     " + _("Start Bitcoin server") + "\n" +
				_("Usage (deprecated, use bitcoin-cli):") + "\n" +
				  "  bitcoind [options] <command> [params]  " + _("Send command to Bitcoin server") + "\n" +
				  "  bitcoind [options] help                " + _("List commands") + "\n" +
				  "  bitcoind [options] help <command>      " + _("Get help for a command") + "\n";

			strUsage += "\n" + HelpMessageCli(false);

			fprintf(stdout, "%s", strUsage.c_str());
			return false;
		}

		// Command-line RPC
		bool fCommandLine = false;
		for (int i = 1; i < argc; i++)
			if (!IsSwitchChar(argv[i][0]) && !boost::algorithm::istarts_with(argv[i], "bitcoin:"))
				fCommandLine = true;

		if (fCommandLine)
		{
			int ret = CommandLineRPC(argc, argv);
			exit(ret);
		}
#ifndef WIN32
		fDaemon = GetBoolArg("-daemon", false);
		if (fDaemon)
		{
			fprintf(stdout, "Bitcoin server starting\n");

			// Daemonize
			pid_t pid = fork();
			if (pid < 0)
			{
				fprintf(stderr, "Error: fork() returned %d errno %d\n", pid, errno);
				return false;
			}
			if (pid > 0) // Parent process, pid is child process id
			{
				CreatePidFile(GetPidFile(), pid);
				return true;
			}
			// Child process falls through to rest of initialization

			pid_t sid = setsid();
			if (sid < 0)
				fprintf(stderr, "Error: setsid() returned %d errno %d\n", sid, errno);
		}
#endif
		SoftSetBoolArg("-server", true);

		detectShutdownThread = new boost::thread(boost::bind(&DetectShutdownThread, &threadGroup));
		fRet = monitor::AppInit(threadGroup);
	}
	catch (std::exception& e) {
		PrintExceptionContinue(&e, "AppInit()");
	} catch (...) {
		PrintExceptionContinue(NULL, "AppInit()");
	}

	if (!fRet)
	{
		if (detectShutdownThread)
			detectShutdownThread->interrupt();

		threadGroup.interrupt_all();
		// threadGroup.join_all(); was left out intentionally here, because we didn't re-test all of
		// the startup-failure cases to make sure they don't result in a hang due to some
		// thread-blocking-waiting-for-another-thread-during-startup case
	}

	if (detectShutdownThread)
	{
		detectShutdownThread->join();
		delete detectShutdownThread;
		detectShutdownThread = NULL;
	}
	monitor::Shutdown();

	return fRet;
}

}

int main(int argc, char* argv[])
{
	bool fRet = false;

	// Connect bitcoind signal handlers
	//noui_connect();

	fRet = monitor::AppInit(argc, argv);

	return (fRet ? 0 : 1);
}

