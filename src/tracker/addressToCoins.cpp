// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "addressToCoins.h"
#include "uint256.h"

#include <stdint.h>

using namespace std;

namespace tracker
{

template <class K, class T,  char _prefix >
class CBatchWrite
{
	insert( K _key, T _object )
	{
		m_batch.Write(make_pair('a', _key), _object);
	}
	CLevelDBBatch & getBatch(){ return m_batch; }
private:
	CLevelDBBatch m_batch;
};



CAddressToCoinsDatabase::CAddressToCoinsDatabase(size_t nCacheSize, bool fMemory, bool fWipe)
: db(GetDataDir() / "addressTocoins",
	nCacheSize,
	fMemory,
	fWipe)
{
}

bool CAddressToCoinsDatabase::getCoinsAmount(const uint160 &_keyId, uint64_t & _amount )
{
	return db.Read(make_pair('a', _keyId), _amount);
}

void static BatchWriteCoinsAmount(CLevelDBBatch &batch, const uint160 &hash, uint64_t const _amount )
{
		batch.Erase(make_pair('a', hash));
		batch.
}

bool CAddressToCoinsDatabase::setCoinsAmount(const uint160 &_keyId, uint64_t const _amount)
{
	return db.Write(make_pair('a', _keyId), _amount);
}

bool CAddressToCoinsDatabase::getCoin(const uint256 &_keyId, uint256 &coins)
{
	return db.Read(make_pair('c', _keyId), coins);
}

bool CAddressToCoinsDatabase::setCoin(const uint256 &_keyId, const uint256 &coins)
{

	return db.Write(make_pair('c', hash), coins);
}

bool CAddressToCoinsDatabase::haveCoin(const uint256 &_keyId)
{
	return db.Exists(make_pair('c', _keyId));
}

bool 
CAddressToCoinsDatabase::batchWrite( CBatchWrite & _batchWrite )
{
	return db.WriteBatch(_batchWrite.getBatch());
}

CAddressToCoins::CAddressToCoins()
:CAddressToCoinsDatabase()
{

}

bool
CAddressToCoins::getCoins( uint160 const &_keyId, std::vector< uint256 > &_coins )
{
	uint256 key = _keyId;
	uint64_t amount;
	if ( !getCoinsAmount(_keyId, amount) )
		return false;

	while( amount-- )
	{
		uint256 coin;
		if ( haveCoin(++key) )
		{
			getCoin( key, coin );
			_coins.push_back( coin );
		}
	}
	return true;
}

bool
CAddressToCoins::setCoins( uint160 const &_keyId, uint256 const & _coin )
{
	uint64_t amount;
	if ( !getCoinsAmount(_keyId, amount) )
		amount = 0;

	if ( !setCoinAmount(_keyId, amount ) )
		return false;

	uint256 key = _keyId;
	if ( !setCoin(++key, _coin ) )
	{
		setCoinAmount(_keyId, --amount );
		return false;
	}
	return true;
}

bool
CAddressToCoins::batchWrite( std::multimap<uint160,uint256> const &mapCoins )
{
	CBatchWrite< uint160, uint64_t, 'a' > amountBatch;
	CBatchWrite< uint256, uint256, 'c' > coinsBatch;
	std::multimap<uint160,uint256> iterator = mapCoins.begin(), end;

	while( iterator != mapCoins.end() )
	{
		end = mapCoins.upper_bound( mapCoins->first );
		uint64_t amount = 0;
		uint256 key = iterator->first;
		while( iterator != end )
		{
			amount++;
			coinsBatch.insert( ++key, iterator->second );
		}
		amountBatch.insert( iterator->first, amount );

		iterator = end;
	}
	batchWrite( amountBatch );
	batchWrite( coinsBatch );
}

bool CAddressToCoinsViewCache::getCoins( uint160 const &_keyId, std::vector< uint256 > &_coins )
{
	std::map<uint256,CCoins>::iterator it = fetchCoins(_keyId);
	assert(it != cacheCoins.end());
	while(it != cacheCoins.end() && it->first == _keyId)
	{
		_coins.push_back( *it->second );
	}
	
	return !_coins.empty();
}

bool CAddressToCoinsViewCache::setCoins( uint160 const &_keyId, uint256 const & _coin )
{
	cacheCoins[_keyId] = coins;
	return true;
}

bool CAddressToCoinsViewCache::haveCoins(const uint160 &txid)
{
	return fetchCoins(txid) != cacheCoins.end();
}

std::map<uint160,uint256>::iterator
CAddressToCoinsViewCache::fetchCoins(const uint160 &_keyId)
{
	std::map<uint160,uint256>::iterator it = cacheCoins.lower_bound(_keyId);
	if (it != cacheCoins.end() && it->first == _keyId)
		return it;

	std::vector< uint256 > tmp;
	if ( !m_addressToCoins->getCoins(_keyId,tmp) )
		return cacheCoins.end();
	BOOST_FOREACH( uint256 & coin, tmp )
	{
		cacheCoins.insert(it, std::make_pair(_keyId, coin));
	}
	return fetchCoins(_keyId);
}

bool 
CAddressToCoinsViewCache::Flush()
{
	bool fOk = base->BatchWrite(cacheCoins, hashBlock);
	if (fOk)
		cacheCoins.clear();
	return fOk;
}

}
bool CCoinsViewDB::BatchWrite(const std::map<uint256, CCoins> &mapCoins, const uint256 &hashBlock) {
	LogPrint("coindb", "Committing %u changed transactions to coin database...\n", (unsigned int)mapCoins.size());

	CLevelDBBatch batch;
	for (std::map<uint256, CCoins>::const_iterator it = mapCoins.begin(); it != mapCoins.end(); it++)
		BatchWriteCoins(batch, it->first, it->second);
	if (hashBlock != uint256(0))
		BatchWriteHashBestChain(batch, hashBlock);

	return db.WriteBatch(batch);
}