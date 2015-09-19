// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BLOCK_INFO_DATABASE_H
#define BLOCK_INFO_DATABASE_H

#include "leveldbwrapper.h"

namespace common
{

class CBlockInfoDatabase
{
	CLevelDBWrapper db;

public:

	bool storeBalance( uint64_t const _timeStamp, uint64_t const _balance );

	bool loadBalances( std::vector< std::pair< uint64_t , uint64_t  > > & _timeBalances );

	bool storeTimeOfFlush( uint64_t const _timeStamp );

	bool loadTimeOfFlush( uint64_t & _timeStamp );

	static CBlockInfoDatabase* getInstance();
private:
	CBlockInfoDatabase( size_t _cacheSize, bool _memory = false, bool _wipe = false );

	static CBlockInfoDatabase * ms_instance;
};

}

#endif // BLOCK_INFO_DATABASE_H
