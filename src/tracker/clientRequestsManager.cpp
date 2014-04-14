// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "clientRequestsManager.h"

#include <boost/foreach.hpp>

namespace tracker
{

uint256 CClientRequestsManager::ms_currentToken = 0;

class CHandleClientRequestVisitor : public boost::static_visitor< void >
{
public:
	CHandleClientRequestVisitor(){};

	void operator()( CTrackerStatsReq const & _transactionStatus )
	{

	}
};

uint256
CClientRequestsManager::addRequest( NodeRequest const & _nodeRequest )
{
	// for transaction may use getHash in  the future??
	boost::lock_guard<boost::mutex> lock( m_lock );
	m_getInfoRequest.insert( std::make_pair( ms_currentToken, _nodeRequest ) );
	return ms_currentToken++;
}

void
CClientRequestsManager::processRequestLoop()
{
	while(1)
	{
		MilliSleep(1000);
		{
			boost::lock_guard<boost::mutex> lock( m_lock );

			BOOST_FOREACH( InfoRequestElement::value_type request, m_getInfoRequest )
			{
				boost::apply_visitor( CHandleClientRequestVisitor(), request.second );
			}
		}
		
	}
}


}
