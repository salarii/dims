// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "common/expireRegister.h"
#include "common/dimsParams.h"
#include "common/actionHandler.h"

namespace common
{

int64_t const
CExpireRegister::m_sleepTime = 1000;

CExpireRegister * CExpireRegister::ms_instance = NULL;

CExpireRegister*
CExpireRegister::getInstance()
{
	if ( !ms_instance )
	{
		ms_instance = new CExpireRegister();
	};
	return ms_instance;
}

void
CExpireRegister::checkForCleanup()
{

	GetTime()
m_nextCleanup

			dimsParams().getCleanupPeriod();

if less than half  before  next  execute

if  after first  check  transactions and  than  execute

}

void
CExpireRegister::loop()
{
	while(1)
	{
		std::vector< uint256 > transactionsHashes;

		if ( m_nextCleanup <= GetTime() )
		{

			std::pair< std::multimap< int64_t, uint256 >::const_iterator, std::multimap< int64_t, uint256 >::const_iterator> hashesRange
					= m_transactions.equal_range( m_nextCleanup );

			std::multimap< int64_t, uint256 >::const_iterator iterator = hashesRange.first;
			while( iterator != hashesRange.second )
			{
				transactionsHashes.push_back( iterator->second );
				++iterator;
			}

			m_transactions.erase( m_nextCleanup );

			m_nextCleanup += dimsParams().getCleanupPeriod();

			if ( transactionsHashes.size() >= dimsParams().getCleanupTriggerPoint() )
			{
				startCleanupAction();
			}
			else
			{
				BOOST_FOREACH( uint256 const & hash, transactionsHashes )
				{
					m_nextCleanup.insert( std::make_pair( m_nextCleanup, hash ) );
				}
			}

		}
		boost::this_thread::interruption_point();

		MilliSleep( m_sleepTime );
	}
}

}
