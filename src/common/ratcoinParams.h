// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef RATCOIN_PARAMS_H
#define RATCOIN_PARAMS_H

#include "bignum.h"
#include "uint256.h"

#include <vector>

#include <boost/thread/mutex.hpp>

using namespace std;

namespace common
{

class CRatcoinParams : public CNetworkParams
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
protected:
    CRatcoinParams() {}

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
};

/**
 * Return the currently selected parameters. This won't change after app startup
 * outside of the unit tests.
 */
const CRatcoinParams &ratcoinParams();

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
    return Params().NetworkID() == CNetworkParams::TESTNET;
}

inline bool RegTest()
{
    return Params().NetworkID() == CNetworkParams::REGTEST;
}

}

#endif // RATCOIN_PARAMS_H