// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "chainparams.h"

#include "assert.h"
#include "core.h"
#include "protocol.h"
#include "util.h"

#include <boost/assign/list_of.hpp>

using namespace boost::assign;



class CMainNetworkParams : public CNetworkParams {
public:
	CMainNetworkParams() {
		// The message start string is designed to be unlikely to occur in normal data.
		// The characters are rarely used upper ASCII, not valid as UTF-8, and produce
		// a large 4-byte int at any alignment.
		pchMessageStart[0] = 0xf9;
		pchMessageStart[1] = 0xbf;
		pchMessageStart[2] = 0xb5;
		pchMessageStart[3] = 0xd9;
		vAlertPubKey = ParseHex("");
		nDefaultPort = 20010;
		nRPCPort = 20011;

		hashGenesisBlock = genesis.GetHash();
/*
		vSeeds.push_back(CDNSSeedData("ratcoin.beacon.org", "dnsseed.ratcoin.beacon.org"));
*/
		base58Prefixes[PUBKEY_ADDRESS] = list_of(58);//R
		base58Prefixes[SCRIPT_ADDRESS] = list_of();//r
		base58Prefixes[SECRET_KEY] =     list_of();//
		base58Prefixes[EXT_PUBLIC_KEY] = list_of()()()();//
		base58Prefixes[EXT_SECRET_KEY] = list_of()()()();//

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
protected:
	vector<CAddress> vFixedSeeds;
};
static CMainParams mainParams;


//
// Testnet (v3)
//
class CTestNetworkParams : public CNetworkParams {
public:
	CTestNetParams() {
		// The message start string is designed to be unlikely to occur in normal data.
		// The characters are rarely used upper ASCII, not valid as UTF-8, and produce
		// a large 4-byte int at any alignment.
		pchMessageStart[0] = 0x0b;
		pchMessageStart[1] = 0x12;
		pchMessageStart[2] = 0x0a;
		pchMessageStart[3] = 0x07;
		vAlertPubKey = ParseHex("04302390343f91cc401d56d68b123028bf52e5fca1939df127f63c6467cdf9c8e2c14b61104cf817d0b780da337893ecc4aaff1309e536162dabbdb45200ca2b0a");
		nDefaultPort = 20020;
		nRPCPort = 20021;
		strDataDir = "testnet3";

		// Modify the testnet genesis block so the timestamp is valid for a later start.
		genesis.nTime = 1296688602;
		genesis.nNonce = 414098458;
		hashGenesisBlock = genesis.GetHash();
		assert(hashGenesisBlock == uint256("0x000000000933ea01ad0ee984209779baaec3ced90fa3f408719526f8d77f4943"));

		vFixedSeeds.clear();
		vSeeds.clear();
		vSeeds.push_back(CDNSSeedData("ratcoin.testnet.beacon.org", "dnsseed.ratcoin.testnet.beacon.org"));

		base58Prefixes[PUBKEY_ADDRESS] = list_of();  // time to set this ??
		base58Prefixes[SCRIPT_ADDRESS] = list_of();
		base58Prefixes[SECRET_KEY]     = list_of();
		base58Prefixes[EXT_PUBLIC_KEY] = list_of()()()();
		base58Prefixes[EXT_SECRET_KEY] = list_of()()()();
	}
	virtual Network NetworkID() const { return CChainParams::TESTNET; }
};
static CTestNetParams testNetParams;

/*
tracer bublic number  t
monitor  public  number  m


m - 0x69
t - 0x7a

r - 0x75
*/

