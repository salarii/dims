// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ADDRESS_TO_COINS_H
#define ADDRESS_TO_COINS_H

#include "leveldbwrapper.h"

namespace tracker
{

uint256
uint160
struct CCoinsStats
{
	uint64_t nTransactions;
	uint64_t nTransactionOutputs;
	uint64_t nSerializedSize;
	uint256 hashSerialized;
	int64_t nTotalAmount;

	CCoinsStats() : nTransactions(0), nTransactionOutputs(0), nSerializedSize(0), hashSerialized(0), nTotalAmount(0) {}
};


class CAddressToCoinsDatabase
{
protected:
	CLevelDBWrapper db;

	CAddressToCoinsDatabase(size_t nCacheSize, bool fMemory = false, bool fWipe = false);

	bool getCoinsAmount(const uint160 &_keyId, uint64_t const _amount );
	bool setCoinsAmount(const uint256 &_keyId, uint64_t & _amount);
	bool getCoin(const uint256 &_keyId, uint256 &coin);
	bool setCoin(const uint256 &_keyId, uint256 &coin);
	bool haveCoin(const uint256 &_keyId);
	bool BatchWrite(const std::map<uint256, CCoins> &mapCoins, const uint256 &hashBlock);
	bool GetStats(CCoinsStats &stats);
};

class CAddressToCoins : public CAddressToCoinsDatabase
{
public:
	CAddressToCoins();
	bool getCoins( uint256 const &_keyId, std::vector< uint256 > &_coins );
	bool setCoins( uint256 const &_keyId, uint256 const & _coin );
};


class CAddressToCoinsViewCache
{
protected:
    CAddressToCoins m_addressToCoins;

    std::multimap<uint160,uint256> cacheCoins;

public:
	CAddressToCoinsViewCache(){};
	bool getCoins( uint256 const &_keyId, std::vector< uint256 > &_coins );
	bool setCoins( uint256 const &_keyId, uint256 const & _coin );
	bool haveCoins( uint256 const &_keyId );

	bool BatchWrite( std::multimap<uint160,uint256> const &mapCoins, uint256 const & hashBlock);
	std::map<uint256,CCoins>::iterator CCoinsViewCache::FetchCoins(const uint256 &txid);
    // Return a modifiable reference to a CCoins. Check HaveCoins first.
    // Many methods explicitly require a CCoinsViewCache because of this method, to reduce
    // copying.
    CCoins &GetCoins(const uint256 &txid);

    // Push the modifications applied to this cache to its base.
    // Failure to call this method before destruction will cause the changes to be forgotten.
    bool Flush();
};

}

#endif //ADDRESS_TO_COINS_H