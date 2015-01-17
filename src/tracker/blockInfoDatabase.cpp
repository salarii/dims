// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


#include "tracker/blockInfoDatabase.h"
#include "util.h"

namespace tracker
{

CBlockInfoDatabase * CBlockInfoDatabase::ms_instance = NULL;

CBlockInfoDatabase*
CBlockInfoDatabase::getInstance()
{
	if ( !ms_instance )
	{
		// parametars like  cache sizes should be stored in some handy place ( ratcoin parameters?? )
		ms_instance = new CBlockInfoDatabase( 1 << 26 );
	};
	return ms_instance;
}

CBlockInfoDatabase::CBlockInfoDatabase( size_t _cacheSize, bool _memory, bool _wipe )
	: db(GetDataDir(common::AppType::Tracker) / "blockInfoDatabase",
		_cacheSize,
		_memory,
		_wipe)
{
}


bool
CBlockInfoDatabase::storeBalance( uint64_t const _timeStamp, uint64_t const _balance )
{
	unsigned int timestumpNumbers = 0;
	if ( !db.Read( 't', timestumpNumbers ) )
		return false;

	timestumpNumbers++;

	if ( !db.Write( std::make_pair('t', timestumpNumbers ), std::make_pair( _timeStamp, _balance ) ) )
		return false;

	return true;
}

bool
CBlockInfoDatabase::loadBalances( std::vector< std::pair< uint64_t , uint64_t > > & _timeBalances )
{
	unsigned int timestumpNumbers = 0;
	if ( !db.Read( 't', timestumpNumbers ) )
		return false;

	for ( unsigned int i = 0; i < timestumpNumbers; ++i )
	{
		std::pair< uint64_t , uint64_t > balance;
		if ( !db.Read( std::make_pair('t', timestumpNumbers ), balance) )
			return false;

		_timeBalances.push_back( balance );
	}

	return true;
}

bool
CBlockInfoDatabase::storeTimeOfFlush( uint64_t const _timeStamp )
{
	if ( !db.Write( 'f', _timeStamp ) )
		return false;

	return true;
}

bool
CBlockInfoDatabase::loadTimeOfFlush( uint64_t & _timeStamp )
{
	if ( !db.Read( 'f', _timeStamp ) )
		return false;

	return true;
}


}
