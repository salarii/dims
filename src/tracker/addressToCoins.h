// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ADDRESS_TO_COINS_H
#define ADDRESS_TO_COINS_H

#include "leveldbwrapper.h"
#include "uint256.h"

namespace tracker
{

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

	CAddressToCoinsDatabase(size_t _cacheSize, bool fMemory = false, bool fWipe = false);

	bool getCoinsAmount( uint160 const &_keyId, uint64_t & _amount );
	bool setCoinsAmount( uint160 const &_keyId, uint64_t const _amount);
	bool getCoin(uint256 const &_keyId, uint256 &coin);
	bool setCoin(uint256 const &_keyId, uint256 const &coin);
	bool haveCoin(uint256 const &_keyId);
	template< class Batch >
	bool batchWrite( Batch& _batchWrite );
	bool GetStats(CCoinsStats &stats);
};

template< class Batch >
bool 
CAddressToCoinsDatabase::batchWrite( Batch & _batchWrite )
{
	return db.WriteBatch(_batchWrite.getBatch());
}

class CAddressToCoins : public CAddressToCoinsDatabase
{
public:
	CAddressToCoins(size_t _cacheSize);
	bool getCoins( uint160 const &_keyId, std::vector< uint256 > &_coins );
	bool setCoins( uint160 const &_keyId, uint256 const & _coin );

	bool batchWrite( std::multimap<uint160,uint256> const &mapCoins );
};


class CAddressToCoinsViewCache
{
protected:
    CAddressToCoins m_addressToCoins;

    std::multimap<uint160,uint256> cacheCoins;

public:
	bool getCoins( uint160 const &_keyId, std::vector< uint256 > &_coins );
	bool setCoins( uint160 const &_keyId, uint256 const & _coin );
	bool haveCoins( uint160 const &_keyId );

	bool flush();

	static CAddressToCoinsViewCache* getInstance();
private:
	CAddressToCoinsViewCache(size_t _cacheSize = 1 << 26):m_addressToCoins(_cacheSize){};

	static CAddressToCoinsViewCache * ms_instance;
	std::map<uint160,uint256>::iterator fetchCoins( uint160 const &_keyId);
};

}

#endif //ADDRESS_TO_COINS_H
