// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "addressToCoins.h"

#include <boost/foreach.hpp>
//#include <stdint.h>

namespace tracker
{

template <class K, class T >
class CBatchWrite
{
public:
	void insert( K const & _key, unsigned char _bucket, T const & _object )
	{
		m_batch.Write( std::make_pair( _bucket, _key ), _object );
	}
	CLevelDBBatch & getBatch(){ return m_batch; }
private:
	CLevelDBBatch m_batch;
};



CAddressToCoinsDatabase::CAddressToCoinsDatabase(size_t nCacheSize, bool fMemory, bool fWipe)
: db(GetDataDir(common::AppType::Tracker) / "addressTocoins",
	nCacheSize,
	fMemory,
	fWipe)
{
}

bool
CAddressToCoinsDatabase::getCoinsAmount( uint160 const &_keyId, char unsigned _bucket, uint64_t & _amount )
{
	return db.Read( std::make_pair( _bucket, _keyId ), _amount );
}

bool
CAddressToCoinsDatabase::setCoinsAmount( uint160 const &_keyId, char unsigned _bucket, uint64_t const _amount )
{
	return db.Write( std::make_pair( _bucket, _keyId ), _amount );
}

bool
CAddressToCoinsDatabase::haveCoinAmount( uint160 const &_keyId, char unsigned _bucket )
{
	return db.Exists( std::make_pair( _bucket, _keyId ) );
}

bool
CAddressToCoinsDatabase::getCoin( CKeyType const &_keyId, char unsigned _bucket, uint256 &coins )
{
	return db.Read( std::make_pair( _bucket, _keyId ), coins );
}

bool
CAddressToCoinsDatabase::setCoin( CKeyType const &_keyId, char unsigned _bucket, uint256 const &coins )
{
	return db.Write( std::make_pair( _bucket, _keyId ), coins );
}

bool
CAddressToCoinsDatabase::eraseCoin( uint256 const &_keyId, char unsigned _bucket )
{
	return db.Erase( std::make_pair( _bucket, _keyId ) );
}

bool
CAddressToCoinsDatabase::haveCoin( uint160 const &_keyId, char unsigned _bucket )
{
	return db.Exists( std::make_pair( _bucket, _keyId ) );
}

void
CAddressToCoinsDatabase::clearView()
{
	return db.clear();
}

bool
CAddressToCoinsDatabase::haveCoin( CKeyType const &_keyId, char unsigned _bucket )
{
	return db.Exists( std::make_pair( _bucket, _keyId ) );
}

CAddressToCoins::CAddressToCoins( size_t _cacheSize)
	:CAddressToCoinsDatabase( _cacheSize, false, false )
{

}

bool CAddressToCoins::getCoinsAmount( uint160 const &_keyId, uint64_t & _amount )
{
	_amount = 0;

	uint64_t tmp = 0;
	CKeyType key( _keyId );

	for ( int unsigned bucket = 0; bucket < getBucketSize() ; ++bucket )
	{
		if ( !haveCoinAmount( _keyId, bucket ) )
			continue;

		if ( !CAddressToCoinsDatabase::getCoinsAmount( _keyId, bucket, tmp ) )
			return false;

		if ( !tmp )
			continue;

		for ( uint64_t id = 0; id < tmp; ++id )
		{
			if ( haveCoin(key + id, bucket ) )
				_amount++;
		}
	}

	return true;
}

bool
CAddressToCoins::getCoins( uint160 const &_keyId, std::vector< uint256 > &_coins )
{
	uint64_t tmp = 0;
	CKeyType key( _keyId );

	for ( int unsigned bucket = 0; bucket < getBucketSize() ; ++bucket )
	{
		if ( !haveCoinAmount( _keyId, bucket ) )
			continue;

		if ( !CAddressToCoinsDatabase::getCoinsAmount( _keyId, bucket, tmp ) )
			return false;

		if ( !tmp )
			continue;

		for ( uint64_t id = 0; id < tmp; ++id )
		{
			CKeyType tmpKey = key + id;

			if ( haveCoin( tmpKey, bucket ) )
			{
				uint256 coin;
				getCoin( tmpKey, bucket,coin );
				_coins.push_back( coin );
			}
		}
	}
	return true;
}

typedef std::map< uint256, uint256 > KeyToCoins;

bool CAddressToCoins::eraseCoin( uint160 const &_keyId, uint256 const & _coin )
{
	unsigned char bucket = getBucket( _coin );

	if ( !haveCoinAmount( _keyId, bucket ) )
		return false;

	uint64_t amount;
	if ( !CAddressToCoinsDatabase::getCoinsAmount( _keyId, bucket, amount ) )
		assert( "!database error" );

	for ( uint64_t id = 0; id < amount; ++id )
	{
		CKeyType tmpKey( _keyId );
		tmpKey += id;

		if ( haveCoin( tmpKey, bucket ) )
		{
			uint256 coin;
			getCoin( tmpKey, bucket, coin );
			if ( coin == _coin )
			{
				return CAddressToCoinsDatabase::eraseCoin( tmpKey, bucket );
			}
		}
	}
	return false;
}

bool
CAddressToCoins::batchWrite( std::multimap<uint160,uint256> const &mapCoins )
{
	CBatchWrite< uint160, uint64_t > amountBatch;
	CBatchWrite< uint256, uint256 > coinsBatch;
	std::multimap< uint160, uint256 >::const_iterator iterator = mapCoins.begin(), end;

	while( iterator != mapCoins.end() )
	{
		end = mapCoins.upper_bound( iterator->first );
		uint64_t amount = 0;
		CKeyType key( iterator->first );

		std::map< unsigned char, uint64_t > amounts;
		std::map< unsigned char, uint64_t >::const_iterator amountIterator;
		std::map< std::pair< uint256, unsigned char >, uint256 > m_recentlyIncluded;
		while( iterator != end )
		{
			char unsigned bucket = getBucket( iterator->second );

			amountIterator = amounts.find( bucket );
			if ( amountIterator != amounts.end() )
				amount =amountIterator->second;
			else
			{
				if ( !CAddressToCoinsDatabase::getCoinsAmount( iterator->first, bucket, amount ) )
					amount = 0;
			}
			int empty = -1;

			bool include = true;
			for ( int unsigned i = 0; i < amount ; ++i )
			{
				CKeyType tmpKey = key + i;

				std::map< std::pair< uint256, unsigned char >, uint256 >::const_iterator recentIterator =
						m_recentlyIncluded.find( std::make_pair( tmpKey, bucket ) );

				if ( recentIterator != m_recentlyIncluded.end() )
				{
					if ( recentIterator->second == iterator->second )
					{
						include = false;
						break;
					}

				}
				else if( !haveCoin( tmpKey, bucket ) )
				{
					empty = i;
				}
				else
				{
					uint256 coin;
					if( getCoin( tmpKey, bucket, coin ) )
					{
						if ( coin == iterator->second )
						{
							include = false;
							break;
						}
					}
					else
					{
						assert( !"can't get present coins" );
					}
				}
			}

			if ( include )
			{
				CKeyType tmpKey;

				if ( empty != -1 )
				{
					tmpKey = key + empty;
					coinsBatch.insert( tmpKey, bucket, iterator->second );
				}
				else
				{
					tmpKey = key + amount;
					coinsBatch.insert( tmpKey, bucket, iterator->second );
					amount++;
				}
				m_recentlyIncluded.insert( std::make_pair(std::make_pair( tmpKey, bucket ), iterator->second ) );

				amountBatch.insert( iterator->first, bucket, amount );
				amounts.insert( std::make_pair( bucket, amount ) );
			}

			iterator++;
		}


	}

	CAddressToCoinsDatabase::batchWrite( amountBatch );
	CAddressToCoinsDatabase::batchWrite( coinsBatch );

	return true;
}

void
CAddressToCoins::clearView()
{
	CAddressToCoinsDatabase::clearView();
}

int unsigned
CAddressToCoins::getBucketSize()
{
	return 256;
}

char unsigned
CAddressToCoins::getBucket( uint256 const & _coins )
{
	return (char unsigned)_coins.GetLow64() % getBucketSize();
}

CAddressToCoinsViewCache * CAddressToCoinsViewCache::ms_instance = NULL;

CAddressToCoinsViewCache*
CAddressToCoinsViewCache::getInstance()
{
	if ( !ms_instance )
	{
		ms_instance = new CAddressToCoinsViewCache();
	};
	return ms_instance;
}

bool CAddressToCoinsViewCache::getCoins( uint160 const &_keyId, std::vector< uint256 > &_coins )
{
	_coins.clear();

	boost::lock_guard<boost::mutex> lock( m_cacheLock );

	std::map<uint160,uint256>::iterator it = fetchCoins(_keyId);

	while( it != m_cacheCoins.upper_bound( _keyId ) )
	{
		_coins.push_back( it->second );
		it++;
	}
	
	return !_coins.empty();
}

bool CAddressToCoinsViewCache::setCoins( uint160 const &_keyId, uint256 const & _coin )
{
	boost::lock_guard<boost::mutex> lock( m_cacheLock );

	m_insertCacheCoins.insert( std::make_pair( _keyId, _coin ) );
	return true;
}

bool CAddressToCoinsViewCache::haveCoins(const uint160 &txid)
{
	boost::lock_guard<boost::mutex> lock( m_cacheLock );

	return fetchCoins(txid) != m_cacheCoins.end();
}

std::map<uint160,uint256>::iterator
CAddressToCoinsViewCache::fetchCoins(const uint160 &_keyId, bool secondPass )
{
	std::map<uint160,uint256>::iterator it = m_cacheCoins.lower_bound(_keyId);

	// expect that if  cache  has it  cache is  right
	if ( it != m_cacheCoins.upper_bound(_keyId) )
	{
		unsigned int distance = std::distance( it, m_cacheCoins.upper_bound(_keyId));

		uint64_t amount;
// slow  only for  debug purposses
		{
		assert( m_addressToCoins.getCoinsAmount( _keyId, amount ) );

		assert( distance == amount );
			return it;
		}
	}
	std::vector< uint256 > tmp;

	if ( !m_addressToCoins.getCoins( _keyId, tmp ) || tmp.empty() )
		return m_cacheCoins.end();
	
	BOOST_FOREACH( uint256 & coin, tmp )
	{
			m_cacheCoins.insert(std::make_pair(_keyId, coin));
	}
	return fetchCoins(_keyId, true);

}

bool
CAddressToCoinsViewCache::eraseCoins( uint160 const &_keyId, uint256 const & _coin )
{
	boost::lock_guard<boost::mutex> lock( m_cacheLock );

	std::multimap<uint160,uint256>::iterator iterator = m_cacheCoins.lower_bound( _keyId );

	while( iterator != m_cacheCoins.upper_bound( _keyId ) )
	{
		if ( iterator->second == _coin )
			m_cacheCoins.erase( _keyId );

		iterator++;
	}

	m_addressToCoins.eraseCoin( _keyId, _coin );
	return true;
}

bool 
CAddressToCoinsViewCache::flush()
{
	boost::lock_guard<boost::mutex> lock( m_cacheLock );

	bool ok = m_addressToCoins.batchWrite( m_insertCacheCoins );

	if (ok)
	{
		for( std::multimap<uint160,uint256>::const_iterator it = m_insertCacheCoins.begin(), end = m_insertCacheCoins.end(); it != end;)
		{
			std::multimap<uint160,uint256>::const_iterator up = m_insertCacheCoins.upper_bound( it->first );
			std::set< uint256 > alreadyUsed;

			for( std::multimap<uint160,uint256>::const_iterator cacheIt = m_cacheCoins.lower_bound( it->first ), cacheEnd = m_cacheCoins.upper_bound( it->first ); cacheIt != cacheEnd; ++cacheIt )
			{
				alreadyUsed.insert( cacheIt->second );
			}

			while( it != up )
			{
				if ( alreadyUsed.find( it->second ) == alreadyUsed.end() )
				{
					alreadyUsed.insert( it->second );
					m_cacheCoins.insert( *it );
				}
				it++;
			}
		}

		m_insertCacheCoins.clear();
	}
	return ok;
}

void
CAddressToCoinsViewCache::clearView()
{
	boost::lock_guard<boost::mutex> lock( m_cacheLock );

	m_cacheCoins.clear();
	m_insertCacheCoins.clear();
	m_addressToCoins.clearView();
}

CAddressToCoinsViewCache::~CAddressToCoinsViewCache()
{
	ms_instance = 0;
}

}
