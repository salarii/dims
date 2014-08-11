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

#include "clientFilters.h"

namespace client
{
const unsigned DnsAskLoopTime = 20;//seconds
const unsigned NetworkAskLoopTime = 20;//seconds

struct CMonitorPresent;
struct CWithoutMonitor;
struct CRecognizeNetwork;

struct CClientUnconnected : boost::statechart::state< CClientUnconnected, CConnectAction >
{
	CClientUnconnected( my_context ctx ) : my_base( ctx )
	{
		context< CConnectAction >().setRequest( new CDnsInfoRequest() );
		m_lastAskTime = GetTime();
	}
	boost::statechart::result react( common::CContinueEvent const & _continueEvent )
	{
		int64_t time = GetTime();
		if ( time - m_lastAskTime < DnsAskLoopTime )
		{
			context< CConnectAction >().setRequest( new common::CContinueReqest<NodeResponses>(uint256(), new CMediumClassFilter( common::RequestKind::Seed ) ) );
		}
		else
		{
			m_lastAskTime = time;
			context< CConnectAction >().setRequest( new CDnsInfoRequest() );
		}
	}

	boost::statechart::result react( CDnsInfo const & _dnsInfo )
	{
		if ( _dnsInfo.m_addresses.empty() )
		{
			context< CConnectAction >().setRequest( new common::CContinueReqest<NodeResponses>(uint256(), new CMediumClassFilter( common::RequestKind::Seed ) ) );
		}
		else
		{
			BOOST_FOREACH( CAddress const & address, _dnsInfo.m_addresses )
			{
				CTrackerLocalRanking::getInstance()->addUnidentifiedNode( common::CUnidentifiedStats( address.ToStringIP(), address.GetPort() ) );
			}
			return transit< CRecognizeNetwork >();
		}
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CContinueEvent >,
	boost::statechart::custom_reaction< CDnsInfo >
	> reactions;

	int64_t m_lastAskTime;
};

struct CRecognizeNetwork : boost::statechart::state< CRecognizeNetwork, CConnectAction >
{
	CRecognizeNetwork( my_context ctx ) : my_base( ctx )
	{
		context< CConnectAction >().setRequest( new CRecognizeNetworkRequest() );

		m_lastAskTime = GetTime();
	}

	boost::statechart::result react( common::CContinueEvent const & _continueEvent )
	{
		int64_t time = GetTime();
		if ( time - m_lastAskTime < NetworkAskLoopTime )
		{
			// second parameter is problematic, maybe this  should  be  indicator  of  very  specific connection
			context< CConnectAction >().setRequest( new common::CContinueReqest<NodeResponses>(uint256(), new CMediumClassFilter( common::RequestKind::Unknown ) ) );
		}
		else
		{
			bool moniorPresent = false;

			if ( !m_uniqueNodes.size() )
			{
				return transit< CRecognizeNetwork >();
			}

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

	boost::statechart::result react( common::CErrorEvent const & _networkInfo )
	{
		int64_t time = GetTime();
		if ( time - m_lastAskTime >= NetworkAskLoopTime )
		{
			context< CConnectAction >().process_event( common::CContinueEvent(uint256() ) );
		}

		context< CConnectAction >().setRequest( new common::CContinueReqest<NodeResponses>(uint256(), new CMediumClassFilter( common::RequestKind::Unknown ) ) );
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CContinueEvent >,
	boost::statechart::custom_reaction< common::CNetworkInfoEvent >,
	boost::statechart::custom_reaction< common::CErrorEvent >
	> reactions;

	std::set< common::CValidNodeInfo > m_uniqueNodes;

	// replace  those  tricks  by  real  time  getTime()
	int64_t m_lastAskTime;
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

		context< CConnectAction >().setRequest( new CTrackersInfoRequest( trackerInfoProfile, new CMediumClassFilter( common::RequestKind::UndeterminedTrackers ) ) );
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
	initiate();
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
