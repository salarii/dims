// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "addressToCoins.h"
#include "uint256.h"

#include <stdint.h>

using namespace std;

namespace tracker
{

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
		batch.Write(make_pair('a', hash), _amount);
}

bool CAddressToCoinsDatabase::setCoinsAmount(const uint256 &_keyId, uint64_t const _amount)
{
	CLevelDBBatch batch;
	BatchWriteCoinsAmount(batch, _keyId, _amount);
	return db.WriteBatch(batch);
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

bool CAddressToCoinsDatabase::BatchWrite(const std::map<uint256, uint256> &mapCoins, const uint256 &hashBlock) {
	LogPrint("coindb", "Committing %u changed transactions to coin database...\n", (unsigned int)mapCoins.size());

	CLevelDBBatch batch;
	for (std::map<uint256, uint256>::const_iterator it = mapCoins.begin(); it != mapCoins.end(); it++)
		BatchWriteCoins(batch, it->first, it->second);

	return db.WriteBatch(batch);
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

bool CAddressToCoinsViewCache::getCoins( uint256 const &_keyId, std::vector< uint256 > &_coins )
{
	std::map<uint256,CCoins>::iterator it = fetchCoins(_keyId);
	assert(it != cacheCoins.end());
	return it->second;
}

bool CAddressToCoinsViewCache::setCoins( uint256 const &_keyId, uint256 const & _coin )
{
	cacheCoins[txid] = coins;
	return true;
}

bool CAddressToCoinsViewCache::haveCoins(const uint256 &txid)
{
	return fetchCoins(txid) != cacheCoins.end();
}

}
