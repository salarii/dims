// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "chainparams.h"
#include "dimsParams.h"

#include "assert.h"
#include "core.h"
#include "protocol.h"
#include "util.h"

#include <boost/assign/list_of.hpp>

using namespace boost::assign;

namespace common
{

AppType::Enum CDimsParams::m_appType;

class CMainDimsParams : public CDimsParams
{
public:
	CMainDimsParams() {

		m_defaultClientPort = 0x1000;
		unsigned int pnSeed[] ={};
		// The message start string is designed to be unlikely to occur in normal data.
		// The characters are rarely used upper ASCII, not valid as UTF-8, and produce
		// a large 4-byte int at any alignment.
		pchMessageStart[0] = 0xf9;
		pchMessageStart[1] = 0xbf;
		pchMessageStart[2] = 0xb5;
		pchMessageStart[3] = 0xd9;
		vAlertPubKey = ParseHex("");
		nDefaultPort = 20010;
		nRPCPort = 20100;

		vSeeds.push_back(CDNSSeedData("vps.dims.red", "seed.dims.red"));

		base58Prefixes[PUBKEY_ADDRESS] = list_of(30);
		base58Prefixes[SCRIPT_ADDRESS] = list_of(30);
		base58Prefixes[SECRET_KEY] =     list_of(135);//
		base58Prefixes[EXT_PUBLIC_KEY] = list_of(0x04)(0x88)(0xB2)(0x1E);//
		base58Prefixes[EXT_SECRET_KEY] = list_of(0x04)(0x88)(0xB2)(0x1E);//

		nodesPrefixes[NodePrefix::Tracker] = list_of(65);
		nodesPrefixes[NodePrefix::Monitor] = list_of(50);
		nodesPrefixes[NodePrefix::Seed] = list_of(63);
		// Convert the pnSeeds array into usable address objects.
		for (unsigned int i = 0; i < ARRAYLEN(pnSeed); i++)
		{
			// It'll only connect to one or two seed nodes because once it connects,
			// it'll get a pile of addresses with newer timestamps.
			// Seed nodes are given a random 'last seen time' of between one and two
			// weeks ago.
			const int64_t nOneWeek = 7*24*60*60;
			struct in_addr ip;
			memcpy(&ip, &pnSeed[i], sizeof(ip));
			CAddress addr(CService(ip, GetDefaultPort()));
			addr.nTime = GetTime() - GetRand(nOneWeek) - nOneWeek;
			vFixedSeeds.push_back(addr);
		}
	}

		vector<CPubKey> const & getPreferedMonitorsAddresses() const
		{
			return m_prefferedMonitors;
		}

	virtual Network NetworkID() const { return CNetworkParams::MAIN; }

	virtual const vector<CAddress>& FixedSeeds() const {
		return vFixedSeeds;
	}

    virtual unsigned int getDefaultClientPort() const
    {
        return m_defaultClientPort;
    }
protected:
	vector<CAddress> vFixedSeeds;
	unsigned int m_defaultClientPort;
	vector<CPubKey> m_prefferedMonitors;// vital parameter !!!
};
static CMainDimsParams mainParams;


//
// Testnet (v3)
//
class CTestDimsParams : public CMainDimsParams {
public:
	CTestDimsParams() {

		m_defaultClientPort = 0x1400;
		// The message start string is designed to be unlikely to occur in normal data.
		// The characters are rarely used upper ASCII, not valid as UTF-8, and produce
		// a large 4-byte int at any alignment.
		pchMessageStart[0] = 0x0b;
		pchMessageStart[1] = 0x12;
		pchMessageStart[2] = 0x0a;
		pchMessageStart[3] = 0x07;
		vAlertPubKey = ParseHex("04302390343f91cc401d56d68b123028bf52e5fca1939df127f63c6467cdf9c8e2c14b61104cf817d0b780da337893ecc4aaff1309e536162dabbdb45200ca2b0a");
		nDefaultPort = 20020;
		strDataDir = "testnet3";

		vFixedSeeds.clear();
		vSeeds.clear();
		vSeeds.push_back(CDNSSeedData("vps.dims.red", "seed.dims.red"));

		base58Prefixes[PUBKEY_ADDRESS] = list_of(90);
		base58Prefixes[SCRIPT_ADDRESS] = list_of(90);
		base58Prefixes[SECRET_KEY]     = list_of(137);
		base58Prefixes[EXT_PUBLIC_KEY] = list_of(0x04)(0x88)(0xB2)(0x1E);
		base58Prefixes[EXT_SECRET_KEY] = list_of(0x04)(0x88)(0xB2)(0x1E);

		nodesPrefixes[NodePrefix::Tracker] = list_of(127);
		nodesPrefixes[NodePrefix::Monitor] = list_of(110);
		nodesPrefixes[NodePrefix::Seed] = list_of(125);

	}

	virtual CNetworkParams::Network NetworkID() const { return CNetworkParams::TESTNET; }
};

static CTestDimsParams testNetParams;

static CMainDimsParams *pCurrentParams = &mainParams;

CNetworkParams const &
CDimsParams::getNetworkParameters()
{
	return *pCurrentParams;
}

CDimsParams const &dimsParams()
{
	return *pCurrentParams;
}

std::string
CDimsParams::getDefaultDirectory() const
{
	assert( m_defaultDirectory.find(convertAppType( getAppType() )) != m_defaultDirectory.end() );
	return m_defaultDirectory.find(convertAppType( getAppType() ))->second;
};

bool 
SelectDimsParamsFromCommandLine()
{
	bool fRegTest = GetBoolArg("-regtest", false);
	bool fTestNet = GetBoolArg("-testnet", false);

	if (fTestNet && fRegTest)
	{
		return false;
	}

	if (fRegTest)
	{
		SelectDimsParams(CNetworkParams::REGTEST);
	}
	else if (fTestNet)
	{
		SelectDimsParams(CNetworkParams::TESTNET);
	}
	else
	{
		SelectDimsParams(CNetworkParams::MAIN);
	}
	return true;
}

void 
SelectDimsParams(CNetworkParams::Network network)
{
	switch (network)
	{
	case CNetworkParams::MAIN:
		pCurrentParams = &mainParams;
		break;
	case CNetworkParams::TESTNET:
	case CNetworkParams::REGTEST:
		pCurrentParams = &testNetParams;
		break;
	default:
		assert(false && "Unimplemented network");
		return;
	}
}

TargetType::Enum
convertAppType( AppType::Enum _appType )
{
#ifdef WIN32
	switch ( _appType )
	{
	case AppType::Client :
		return TargetType::ClientWindows;
	case AppType::Tracker:
		return TargetType::TrackerWindows;
	case AppType::Monitor:
		return TargetType::MonitorWindows;
	case AppType::Seed:
		return TargetType::SeedWindows
	default:
		break;
	}
#else
#ifdef MAC_OSX
	switch ( _appType )
	{
	case AppType::Client :
		return TargetType::ClientMac;
	case AppType::Tracker:
		return TargetType::TrackerMac;
	case AppType::Monitor:
		return TargetType::MonitorMac;
	case AppType::Seed:
		return TargetType::SeedMac;
	default:
		break;
	}
#else
	switch ( _appType )
	{
	case AppType::Client :
		return TargetType::ClientLinux;
	case AppType::Tracker:
		return TargetType::TrackerLinux;
	case AppType::Monitor:
		return TargetType::MonitorLinux;
	case AppType::Seed:
		return TargetType::SeedLinux;
	default:
		break;
	}
#endif
#endif
	assert( !"not recognized" );
	return TargetType::SeedLinux;
}

}

