// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "connectAction.h"
#include "clientResponses.h"
#include "controlRequests.h"

#include "common/setResponseVisitor.h"
#include "common/commonEvents.h"

#include <boost/statechart/custom_reaction.hpp>
#include <boost/statechart/state_machine.hpp>
#include <boost/statechart/state.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/event.hpp>

namespace client
{
const unsigned DnsAskLoopCounter = 10;
//
struct CClientUnconnected : boost::statechart::state< CClientUnconnected, CConnectAction >
{
	CClientUnconnected( my_context ctx ) : my_base( ctx )
	{
		context< CConnectAction >().setRequest( new CDnsInfoRequest() );
	}
	boost::statechart::result react( common::CContinueEvent const & _continueEvent )
	{
		m_counter--;
		context< CConnectAction >().setRequest( new common::CContinueReqest<NodeResponses>(uint256(), common::RequestKind::NetworkInfo ) );

		if ( !m_counter )
			context< CConnectAction >().setRequest( new CDnsInfoRequest() );
	}

	boost::statechart::result react( CDnsInfo const & _dnsInfo )
	{

	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CContinueEvent >,
	boost::statechart::custom_reaction< CDnsInfo >
	> reactions;

	unsigned int m_counter;
};

struct CRecognizeNetwork : boost::statechart::state< CRecognizeNetwork, CConnectAction >
{
	CRecognizeNetwork( my_context ctx ) : my_base( ctx )
	{
	}

	boost::statechart::result react( common::CContinueEvent const & _continueEvent )
	{
		if ( m_counter-- )
		{
			// second parameter is problematic, maybe this  should  be  indicator  of  very  specific connection
			context< CConnectAction >().setRequest( new common::CContinueReqest<NodeResponses>(uint256(), common::RequestKind::NetworkInfo ) );
		}
		else
		{

		}
	}

	boost::statechart::result react( common::CNetworkInfoEvent const & _networkInfo )
	{
		BOOST_FOREACH( common::CValidNodeInfo const & validNode, _networkInfo.m_networkInfo )
		{
			m_uniqueNodes.insert( validNode );
		}
	}

	void analyseData()
	{
		BOOST_FOREACH( common::CValidNodeInfo const & validNode, m_uniqueNodes )
		{
			//m_uniqueNodes.insert( validNodes );
		}
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CContinueEvent >,
	boost::statechart::custom_reaction< common::CNetworkInfoEvent >
	> reactions;

	std::set< common::CValidNodeInfo > m_uniqueNodes;
	unsigned int m_counter;
};

CConnectAction::CConnectAction()
:m_request( 0 )
{
}

void
CConnectAction::accept( common::CSetResponseVisitor< NodeResponses > & _visitor )
{
	_visitor.visit( *this );
}

common::CRequest< NodeResponses >*
CConnectAction::execute()
{
	return m_request;
}

void
CConnectAction::setRequest( common::CRequest< NodeResponses >* _request )
{
	m_request = _request;
}

}
