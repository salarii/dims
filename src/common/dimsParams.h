// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef RATCOIN_PARAMS_H
#define RATCOIN_PARAMS_H

#include "bignum.h"
#include "uint256.h"
#include "networksParameters.h"
#include <vector>

#include <boost/thread/mutex.hpp>
#include <map>
#include <boost/assign.hpp>

using namespace std;

namespace common
{
struct AppType
{
	enum Enum
	{
		  Client = 0
		, Tracker
		, Monitor
		, Seed
		, SizeOfEnum
	};
};

struct TargetType
{
	enum Enum
	{
		  ClientWindows
		, TrackerWindows
		, MonitorWindows
		, SeedWindows
		, ClientLinux
		, TrackerLinux
		, MonitorLinux
		, SeedLinux
		, ClientMac
		, TrackerMac
		, MonitorMac
		, SeedMac
	};
};


class CDimsParams : public CNetworkParams
{
public:
    const uint256& HashOfBitcoinGenesisBlock() const { return hashGenesisBlock; }
    const MessageStartChars& MessageStart() const { return pchMessageStart; }
    const vector<unsigned char>& AlertKey() const { return vAlertPubKey; }
     int GetDefaultPort() const { return nDefaultPort; }
    virtual bool RequireRPCPassword() const { return true; }
    const string& DataDir() const { return strDataDir; }
    virtual Network NetworkID() const = 0;
    const vector<CDNSSeedData>& DNSSeeds() const { return vSeeds; }
    const std::vector<unsigned char> &Base58Prefix(Base58Type type) const { return base58Prefixes[type]; }
    virtual const vector<CAddress>& FixedSeeds() const = 0;
    int RPCPort() const { return nRPCPort; }
    static CNetworkParams const & getNetworkParameters();
	std::string getDefaultDirectory() const;
    virtual unsigned int getDefaultClientPort() const = 0;
	static void setAppType( AppType::Enum const _appType ){ m_appType = _appType; }
	static AppType::Enum getAppType(){ return m_appType; }
protected:


	CDimsParams()
    {
	 m_defaultDirectory = boost::assign::map_list_of< TargetType::Enum, std::string >
			 (TargetType::ClientWindows, "DimsClient")
			 (TargetType::TrackerWindows, "DimsTracker")
			 (TargetType::MonitorWindows, "DimsMonitor")
			 (TargetType::SeedWindows, "DimsSeed")
			 (TargetType::ClientLinux, ".dimsClient")
			 (TargetType::TrackerLinux, ".dimsTracker")
			 (TargetType::MonitorLinux, ".dimsMonitor")
			 (TargetType::SeedLinux, ".dimsSeed")
			 (TargetType::ClientMac, "DimsClient")
			 (TargetType::TrackerMac, "DimsTracker")
			 (TargetType::MonitorMac, "DimsMonitor")
			 (TargetType::SeedMac, "DimsSeed");
	}

    uint256 hashGenesisBlock;
    MessageStartChars pchMessageStart;
    // Raw pub key bytes for the broadcast alert signing key.
    vector<unsigned char> vAlertPubKey;
    int nDefaultPort;
    int nRPCPort;
    CBigNum bnProofOfWorkLimit;
    int nSubsidyHalvingInterval;
    string strDataDir;
    vector<CDNSSeedData> vSeeds;
    std::vector<unsigned char> base58Prefixes[MAX_BASE58_TYPES];
    std::map< TargetType::Enum, std::string > m_defaultDirectory;
	static AppType::Enum m_appType;
};

/**
 * Return the currently selected parameters. This won't change after app startup
 * outside of the unit tests.
 */
CDimsParams const &dimsParams();

/** Sets the params returned by Params() to those for the given network. */
void SelectRatcoinParams(CNetworkParams::Network network);

/**
 * Looks for -regtest or -testnet and then calls SelectParams as appropriate.
 * Returns false if an invalid combination is given.
 */
bool SelectRatcoinParamsFromCommandLine();

inline bool TestNet()
{
    // Note: it's deliberate that this returns "false" for regression test mode.
	return dimsParams().NetworkID() == CNetworkParams::TESTNET;
}

inline bool RegTest()
{
	return dimsParams().NetworkID() == CNetworkParams::REGTEST;
}

TargetType::Enum
convertAppType( AppType::Enum _appType );

}

#endif // RATCOIN_PARAMS_H
