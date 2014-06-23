// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef NETWORKS_PARAMETERS_H
#define NETWORKS_PARAMETERS_H

#include "uint256.h"

struct CDNSSeedData
{
	std::string name, host;
	CDNSSeedData(const std::string &strName, const std::string &strHost) : name(strName), host(strHost) {}
};

class CAddress;

#define MESSAGE_START_SIZE 4
typedef unsigned char MessageStartChars[MESSAGE_START_SIZE];

class CNetworkParams
{
public:
	enum Network {
		MAIN,
		TESTNET,
		REGTEST,

		MAX_NETWORK_TYPES
	};

	enum Base58Type {
		PUBKEY_ADDRESS,
		SCRIPT_ADDRESS,
		SECRET_KEY,
		EXT_PUBLIC_KEY,
		EXT_SECRET_KEY,

		MAX_BASE58_TYPES
	};

public:
    //virtual const uint256& HashOfBitcoinGenesisBlock() const = 0;
    virtual const MessageStartChars& MessageStart() const = 0;
    virtual const std::vector<unsigned char>& AlertKey() const = 0;
    virtual int GetDefaultPort() const = 0;
    virtual bool RequireRPCPassword() const = 0;
    virtual const std::string& DataDir() const = 0;
    virtual Network NetworkID() const = 0;
    virtual const std::vector<CDNSSeedData>& DNSSeeds() const = 0;
    virtual const std::vector<unsigned char> &Base58Prefix(Base58Type type) const = 0;
    virtual const std::vector<CAddress>& FixedSeeds() const = 0;
    virtual int RPCPort() const = 0;
};

template < class T >
CNetworkParams const & 
GetNetworkParams()
{
	return T::getNetworkParameters();
}


#endif // NETWORKS_PARAMETERS_H
