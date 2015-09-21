// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ADDRESS_TO_COINS_H
#define ADDRESS_TO_COINS_H

#include "leveldbwrapper.h"
#include "uint256.h"

namespace tracker
{
// probably the  weakest point of  it all
// there will be  time to  fix  it
struct CCoinsStats
{
	uint64_t nTransactions;
	uint64_t nTransactionOutputs;
	uint64_t nSerializedSize;
	uint256 hashSerialized;
	int64_t nTotalAmount;

	CCoinsStats() : nTransactions(0), nTransactionOutputs(0), nSerializedSize(0), hashSerialized(0), nTotalAmount(0) {}
};

struct CKeyType : public uint256
{
	CKeyType(){}

	CKeyType( uint160 const & _key )
	{
		uint256 temp;
		temp = _key;
		temp <<= 96;
		((uint256 &)*this) = temp;
	}

	CKeyType( uint256 const & _key )
	{
		(uint256&)*this = _key;
	}
};

class CAddressToCoinsDatabase
{
public:
	bool setTransactionInputs( uint256 const &_hash, std::vector< CKeyID > const & _inputs );
	bool getTransactionInputs( uint256 const &_hash, std::vector< CKeyID > & _inputs );

protected:
	CLevelDBWrapper db;

	CAddressToCoinsDatabase(size_t _cacheSize, bool fMemory = false, bool fWipe = false);

	bool getCoinsAmount( uint160 const &_keyId, char unsigned _bucket, uint64_t & _amount );
	bool setCoinsAmount( uint160 const &_keyId, char unsigned _bucket, uint64_t const _amount );
	bool haveCoinAmount( uint160 const &_keyId, char unsigned _bucket );
	bool getCoin( CKeyType const &_keyId, char unsigned _bucket, uint256 & coins );
	bool setCoin( CKeyType const &_keyId, char unsigned _bucket, uint256 const &coins );
	bool eraseCoin( uint256 const &_keyId, char unsigned _bucket );
	bool haveCoin( CKeyType const &_keyId, char unsigned _bucket );
	bool haveCoin(uint160 const &_keyId, char unsigned _bucket );
	bool eraseCoin( uint256 const &_keyId );
	template< class Batch >
	bool batchWrite( Batch& _batchWrite );
	bool GetStats(CCoinsStats &stats);

	void clearView();
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
	CAddressToCoins( size_t _cacheSize);
	bool getCoinsAmount( uint160 const &_keyId, uint64_t & _amount);
	bool getCoins( uint160 const &_keyId, std::vector< uint256 > &_coins );
	bool eraseCoin( uint160 const &_keyId, uint256 const & _coin );
	bool batchWrite( std::multimap<uint160,uint256> const &mapCoins );

	void clearView();
private:
	char unsigned getBucket( uint256 const & _coins );
	int unsigned getBucketSize();
};


class CAddressToCoinsViewCache
{
protected:
	CAddressToCoins m_addressToCoins;
// replace  this cache  multimap it is  inefficient
	std::multimap<uint160,uint256> m_cacheCoins; //control numbers created??
	std::multimap<uint160,uint256> m_insertCacheCoins;
public:
	bool getCoins( uint160 const &_keyId, std::vector< uint256 > &_coins );
	bool setCoins( uint160 const &_keyId, uint256 const & _coin );// flush is required after this
	bool haveCoins( uint160 const &_keyId );
	bool eraseCoins( uint160 const &_keyId, uint256 const & _coin  );
	bool flush();

	bool setTransactionInputs( uint256 const &_hash, std::vector< CKeyID > const & _inputs );
	bool getTransactionInputs( uint256 const &_hash, std::vector< CKeyID > & _inputs );


	static CAddressToCoinsViewCache* getInstance();

	void clearView();

	~CAddressToCoinsViewCache();
private:
	mutable boost::mutex m_cacheLock;

	CAddressToCoinsViewCache( size_t _cacheSize = 1 << 26):m_addressToCoins( _cacheSize){};

	static CAddressToCoinsViewCache * ms_instance;
	std::map<uint160,uint256>::iterator fetchCoins( uint160 const &_keyId, bool secondPass = false );
};

}

#endif //ADDRESS_TO_COINS_H
