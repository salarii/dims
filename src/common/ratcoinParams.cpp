// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "chainparams.h"
#include "ratcoinParams.h"

#include "assert.h"
#include "core.h"
#include "protocol.h"
#include "util.h"

#include <boost/assign/list_of.hpp>

using namespace boost::assign;

namespace common
{

class CMainRatcoinParams : public CRatcoinParams
{
public:
	CMainRatcoinParams() {

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

//		vSeeds.push_back(CDNSSeedData("ratcoin.beacon.org", "dnsseed.ratcoin.beacon.org"));

		base58Prefixes[PUBKEY_ADDRESS] = list_of(60);//R
		base58Prefixes[SCRIPT_ADDRESS] = list_of(60);//r
		base58Prefixes[SECRET_KEY] =     list_of(60);//
		base58Prefixes[EXT_PUBLIC_KEY] = list_of(0x04)(0x88)(0xB2)(0x1E);//
		base58Prefixes[EXT_SECRET_KEY] = list_of(0x04)(0x88)(0xB2)(0x1E);//

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
};
static CMainRatcoinParams mainParams;


//
// Testnet (v3)
//
class CTestRatcoinParams : public CMainRatcoinParams {
public:
	CTestRatcoinParams() {

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
		vSeeds.push_back(CDNSSeedData("ratcoin.testnet.beacon.org", "dnsseed.ratcoin.testnet.beacon.org"));

		base58Prefixes[PUBKEY_ADDRESS] = list_of(0x7a);  // time to set this ??
		base58Prefixes[SCRIPT_ADDRESS] = list_of(0x7a);
		base58Prefixes[SECRET_KEY]     = list_of(0x7a);
		base58Prefixes[EXT_PUBLIC_KEY] = list_of(0x04)(0x88)(0xB2)(0x1E);
		base58Prefixes[EXT_SECRET_KEY] = list_of(0x04)(0x88)(0xB2)(0x1E);
	}
	virtual CNetworkParams::Network NetworkID() const { return CNetworkParams::TESTNET; }
};

static CTestRatcoinParams testNetParams;

static CMainRatcoinParams *pCurrentParams = &mainParams;

CNetworkParams const &
CRatcoinParams::getNetworkParameters()
{
	return *pCurrentParams;
}

CRatcoinParams const &ratcoinParams()
{
	return *pCurrentParams;
}

std::string
CRatcoinParams::getDefaultDirectory(AppType::Enum _targetType ) const
{
	assert( m_defaultDirectory.find(convertAppType( _targetType )) != m_defaultDirectory.end() );
	return m_defaultDirectory.find(convertAppType( _targetType ))->second;
};

bool 
SelectRatcoinParamsFromCommandLine()
{
	bool fRegTest = GetBoolArg("-regtest", false);
	bool fTestNet = GetBoolArg("-testnet", false);

	if (fTestNet && fRegTest)
	{
		return false;
	}

	if (fRegTest)
	{
		SelectRatcoinParams(CNetworkParams::REGTEST);
	}
	else if (fTestNet)
	{
		SelectRatcoinParams(CNetworkParams::TESTNET);
	}
	else
	{
		SelectRatcoinParams(CNetworkParams::MAIN);
	}
	return true;
}

void 
SelectRatcoinParams(CNetworkParams::Network network) 
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
	default:
		break;
	}
#endif
#endif
}

}



/*
tracer bublic number  t
monitor  public  number  m


*/

