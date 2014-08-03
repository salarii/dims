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

#include <boost/assign/list_of.hpp>

namespace client
{
const unsigned DnsAskLoopCounter = 10;


struct CMonitorPresent;
struct CWithoutMonitor;

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
			bool moniorPresent = false;

			BOOST_FOREACH( common::CValidNodeInfo const & validNode, m_uniqueNodes )
			{
				if ( validNode.m_role == common::CRole::Monitor )
				{
					moniorPresent = true;
					CTrackerLocalRanking::getInstance()->addMonitor( common::CNodeStatistic( validNode.m_key, validNode.m_address.ToStringIP(), common::ratcoinParams().getDefaultClientPort() ) );
				}
				else if ( validNode.m_role == common::CRole::Monitor )
				{
					CTrackerLocalRanking::getInstance()->addUndeterminedTracker( common::CNodeStatistic( validNode.m_key, validNode.m_address.ToStringIP(), common::ratcoinParams().getDefaultClientPort() ) );
				}
			}
			return moniorPresent ? transit< CMonitorPresent >() : transit< CWithoutMonitor >();

		}
	}

	boost::statechart::result react( common::CNetworkInfoEvent const & _networkInfo )
	{
		BOOST_FOREACH( common::CValidNodeInfo const & validNode, _networkInfo.m_networkInfo )
		{
			m_uniqueNodes.insert( validNode );
		}
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CContinueEvent >,
	boost::statechart::custom_reaction< common::CNetworkInfoEvent >
	> reactions;

	std::set< common::CValidNodeInfo > m_uniqueNodes;

	// replace  those  tricks  by  real  time  getTime()
	unsigned int m_counter;
};


struct CMonitorPresent : boost::statechart::state< CMonitorPresent, CConnectAction >
{
	CMonitorPresent( my_context ctx ) : my_base( ctx )
	{
	}

//CTrackersInfoRequest
};

struct CWithoutMonitor : boost::statechart::state< CWithoutMonitor, CConnectAction >
{
	CWithoutMonitor( my_context ctx ) : my_base( ctx )
	{
		std::vector< TrackerInfo::Enum > trackerInfoProfile
				= boost::assign::list_of(TrackerInfo::Ip)(TrackerInfo::Price)(TrackerInfo::Rating)(TrackerInfo::PublicKey)(TrackerInfo::MinPrice)(TrackerInfo::MaxPrice);

		context< CConnectAction >().setRequest( new CTrackersInfoRequest( trackerInfoProfile, common::RequestKind::UndeterminedTrackers ) );
	}

	boost::statechart::result react( common::CContinueEvent const & _continueEvent )
	{
	}


	boost::statechart::result react( common::CTrackerStats const & _continueEvent )
	{
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CContinueEvent >,
	boost::statechart::custom_reaction< common::CTrackerStats >
	> reactions;
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
