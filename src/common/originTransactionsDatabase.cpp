// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "boost/foreach.hpp"

#include "common/originTransactionsDatabase.h"

#include "util.h"

namespace common
{

COriginTransactionDatabase * COriginTransactionDatabase::ms_instance = NULL;

COriginTransactionDatabase*
COriginTransactionDatabase::getInstance()
{
	if ( !ms_instance )
	{
		// parametars like  cache sizes should be stored in some handy place ( ratcoin parameters?? )
		ms_instance = new COriginTransactionDatabase( 1 << 26 );
	};
	return ms_instance;
}

COriginTransactionDatabase::COriginTransactionDatabase( size_t _cacheSize, bool _memory, bool _wipe )
	: db(GetDataDir(common::AppType::Tracker) / "blockInfoDatabase",
		_cacheSize,
		_memory,
		_wipe)
{
}

bool
COriginTransactionDatabase::storeOriginTransactionsFlush(
		  uint64_t const _timeStamp
		, uint64_t const _totalBalance
		, std::vector< std::vector< unsigned char > > const & _keys
		, std::vector< uint64_t  > const & _balances )
{
	uint64_t flushNumber;

	getCurrentFlushNumber( (unsigned int & )flushNumber );

	CResourceDescription description;

	description.m_timeStamp = _timeStamp;

	description.m_totalBalance = _totalBalance;

	assert( _keys.size() == _balances.size() );

	BOOST_FOREACH( std::vector< unsigned char > const & key, _keys )
	{
		setKey( ( flushNumber << 32 ) | description.m_keyNumber, _balances.at( description.m_keyNumber ), key );
		description.m_keyNumber++;
	}

	setResourceDescription( flushNumber, description );

	setCurrentFlushNumber( ++flushNumber );

	return true;
}

bool
COriginTransactionDatabase::setKey( uint64_t const _id, uint64_t const _balance, std::vector< unsigned char > const & _key )
{
	if ( !db.Write( std::make_pair('k', _id ), std::make_pair( _key, _balance ) ) )
		return false;

	return true;
}

bool
COriginTransactionDatabase::getKey( uint64_t const _id, uint64_t & _balance, std::vector< unsigned char > & _key )
{
	std::pair< uint64_t , std::vector< unsigned char > > key;
	if ( !db.Read( std::make_pair('k', _id ), key ) )
		return false;

	_balance = key.first;
	_key = key.second;

	return true;
}

bool
COriginTransactionDatabase::getCurrentFlushNumber( unsigned int & _flushNumber )
{
	if ( !db.Read( 'f', _flushNumber ) )
		return false;

	return true;
}

bool
COriginTransactionDatabase::setCurrentFlushNumber( unsigned int const _flushNumber )
{
	if ( !db.Write( 'f', _flushNumber ) )
		return false;

	return true;
}

bool
COriginTransactionDatabase::setResourceDescription( unsigned _id, CResourceDescription const & _resourceDescription )
{
	if ( !db.Write( std::make_pair('r', _id ), _resourceDescription ) )
		return false;

	return true;
}

bool
COriginTransactionDatabase::getResourceDescription( unsigned _id, CResourceDescription & _resourceDescription )
{
	if ( !db.Read( std::make_pair('r', _id ), _resourceDescription ) )
		return false;

	return true;
}

bool
COriginTransactionDatabase::getOriginTransactions(
		uint64_t const _timeStamp
	  , std::vector< std::vector< unsigned char > > & _keys
	  , std::vector< uint64_t > & _balances )
{
	CResourceDescription resourceDescription;

	unsigned int max;
	if ( !getCurrentFlushNumber( max ) )
		return false;

	unsigned int current = --max;

	if ( !current )
		return true;

	while( 1 )
	{
		if ( !getResourceDescription( current - 1, resourceDescription ) )
			return false;

		uint64_t prevTime = resourceDescription.m_timeStamp;

		if ( !getResourceDescription( current, resourceDescription ) )
			return false;

		uint64_t time = resourceDescription.m_timeStamp;

		if ( _timeStamp <= time && _timeStamp >= prevTime )
		{
			do{
				for ( unsigned int i = 0;  ; i++ )
				{
					uint64_t balance;
					std::vector< unsigned char > key;
					uint64_t temp = current;
					if ( !getKey( ( temp << 32 ) | i, balance, key ) )
						return false;

					_keys.push_back( key );
					_balances.push_back( balance );
				}

				current++;
			}while( current <= max );

			//  gather transactions  to  update  and  update  them
		}
		else if ( _timeStamp <= time )
		{
			current = current/2;
		}
		else if ( _timeStamp >= time )
		{
			if ( max == current )
				return true; // no transaction to update

			current = current + ( max - current )  /2;
		}
	}

}

}

