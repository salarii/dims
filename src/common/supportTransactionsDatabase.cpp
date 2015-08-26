// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "common/supportTransactionsDatabase.h"

#include <boost/foreach.hpp>
//#include <stdint.h>

namespace common
{

template <class K, class T, char _prefix >
class CBatchWrite
{
public:
	void insert( K _key, T _object )
	{
		m_batch.Write( std::make_pair(_prefix, _key), _object );
	}
	CLevelDBBatch & getBatch(){ return m_batch; }
private:
	CLevelDBBatch m_batch;
};



CTransactionSpecificData::CTransactionSpecificData(size_t nCacheSize, bool fMemory, bool fWipe)
: db(GetDataDir(common::AppType::Tracker) / "supportTransactions",
	nCacheSize,
	fMemory,
	fWipe)
{
}

bool
CTransactionSpecificData::getTransactionLocation( uint256 const &_hash, uint64_t & _location )
{
	return db.Read( std::make_pair('l', _hash), _location );
}

bool
CTransactionSpecificData::setTransactionLocation( uint256 const &_hash, uint64_t & _location )
{
	return db.Write( std::make_pair('l', _hash), _location );
}

bool
CTransactionSpecificData::eraseTransactionLocation( uint256 const &_hash )
{
	return db.Erase( std::make_pair('l', _hash) );
}

void
CTransactionSpecificData::clearView()
{
	return db.clear();
}

CSupportTransactionsDatabase * CSupportTransactionsDatabase::ms_instance = NULL;

CSupportTransactionsDatabase*
CSupportTransactionsDatabase::getInstance()
{
	if ( !ms_instance )
	{
		ms_instance = new CSupportTransactionsDatabase();
	};
	return ms_instance;
}

bool
CSupportTransactionsDatabase::getTransactionLocation( uint256 const &_hash, uint64_t & _location )
{
	boost::lock_guard<boost::mutex> lock( m_cacheLock );
	std::map<uint256,uint64_t>::iterator iterator = m_transactionToLocationCache.find( _hash );

	if ( iterator != m_transactionToLocationCache.end() )
	{
		_location = iterator->second;
	}

	if ( !m_transactionSpecificData.getTransactionLocation( _hash, _location ) )
		return false;

	m_transactionToLocationCache.insert( std::make_pair( _hash, _location ) );

	return true;
}

bool
CSupportTransactionsDatabase::setTransactionLocation( uint256 const &_hash, uint64_t const _location )
{
	boost::lock_guard<boost::mutex> lock( m_cacheLock );
	m_transactionToLocationCacheInsert.insert( std::make_pair( _hash, _location ) );
	return true;
}

bool
CSupportTransactionsDatabase::eraseTransactionLocation( uint256 const &_hash )
{
	boost::lock_guard<boost::mutex> lock( m_cacheLock );
	bool result = m_transactionToLocationCache.erase( _hash );

	return result ||  m_transactionSpecificData.eraseTransactionLocation( _hash );
}

bool
CSupportTransactionsDatabase::flush()
{
	boost::lock_guard<boost::mutex> lock( m_cacheLock );
	CBatchWrite< uint256, uint64_t, 'l' > locations;

	typedef std::pair<uint256,uint64_t> Element;

	BOOST_FOREACH( Element const & element, m_transactionToLocationCacheInsert )
	{
		locations.insert( element.first, element.second );
	}

	bool ok = m_transactionSpecificData.batchWrite(locations);
	if (ok)
		m_transactionToLocationCacheInsert.clear();
	return ok;
}

void
CSupportTransactionsDatabase::clearView()
{
	boost::lock_guard<boost::mutex> lock( m_cacheLock );
	m_transactionToLocationCacheInsert.clear();
	m_transactionToLocationCache.clear();
	m_transactionSpecificData.clearView();
}

CSupportTransactionsDatabase::~CSupportTransactionsDatabase()
{
	ms_instance = 0;
}

}

