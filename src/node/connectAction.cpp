// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "connectAction.h"
#include "clientResponses.h"
#include "controlRequests.h"
#include "common/mediumRequests.h"

#include "common/setResponseVisitor.h"
#include "common/commonEvents.h"

#include <boost/statechart/custom_reaction.hpp>
#include <boost/statechart/state_machine.hpp>
#include <boost/statechart/state.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/event.hpp>

#include <boost/assign/list_of.hpp>

#include "clientFilters.h"
#include "clientControl.h"
#include "clientEvents.h"

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
		return discard_event();
	}

	boost::statechart::result react( CDnsInfo const & _dnsInfo )
	{
		if ( _dnsInfo.m_addresses.empty() )
		{
			context< CConnectAction >().setRequest( new common::CContinueReqest<NodeResponses>(uint256(), new CMediumClassFilter( common::RequestKind::Seed ) ) );
			return discard_event();
		}
		else
		{
			BOOST_FOREACH( CAddress const & address, _dnsInfo.m_addresses )
			{
				CTrackerLocalRanking::getInstance()->addUnidentifiedNode( address.ToStringIP(), common::CUnidentifiedStats( address.ToStringIP(), address.GetPort() ) );
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

/*
definitely total crap
???
nodes should be  asked about ip-s first???
all the rest info about  node  should  be gotten  from given node
create error  handlig  functionality
in cases where some of  nodes  are  out of reach
??
*/
struct CRecognizeNetwork : boost::statechart::state< CRecognizeNetwork, CConnectAction >
{
	CRecognizeNetwork( my_context ctx ) : my_base( ctx )
	{
		context< CConnectAction >().setRequest( new CRecognizeNetworkRequest() );

		m_lastAskTime = GetTime();
	}

	boost::statechart::result react( common::CPending const & _pending )
	{
		m_nodeToToken.insert( std::make_pair( _pending.m_networkPtr, _pending.m_token ) );

		m_pending.insert( _pending.m_networkPtr );

		int64_t time = GetTime();
		if ( time - m_lastAskTime < NetworkAskLoopTime )
		{
			if ( !context< CConnectAction >().isRequestReady() )
					context< CConnectAction >().setRequest( new CInfoRequestContinueComplex( m_nodeToToken, new CSpecificMediumFilter( m_pending ) ) );
			return discard_event();
		}
		else
		{

			if ( !m_uniqueNodes.size() )
			{
				return transit< CRecognizeNetwork >();
			}

			bool moniorPresent = false;

			analyseData( moniorPresent );

			return moniorPresent ? transit< CMonitorPresent >() : transit< CWithoutMonitor >();
		}
	}

	boost::statechart::result react( common::CClientNetworkInfoEvent const & _networkInfo )
	{
		m_pending.erase( _networkInfo.m_nodeIndicator );

		common::CNodeStats validNodeInfo( _networkInfo.m_selfKey, _networkInfo.m_ip, common::dimsParams().getDefaultClientPort() );

		if ( _networkInfo.m_selfRole == common::CRole::Monitor )
		{
			CTrackerLocalRanking::getInstance()->addMonitor( _networkInfo.m_ip, validNodeInfo );
		}
		else if ( _networkInfo.m_selfRole == common::CRole::Tracker )
		{
			CTrackerLocalRanking::getInstance()->addUndeterminedTracker( _networkInfo.m_ip, validNodeInfo );
		}

		BOOST_FOREACH( common::CValidNodeInfo const & validNode, _networkInfo.m_networkInfo )
		{
			m_uniqueNodes.insert( validNode );
		}

		if ( !m_pending.size() )
		{
			if ( !m_uniqueNodes.size() )
			{
				return transit< CWithoutMonitor >(); // not  ok
			}

			bool moniorPresent = false;

			analyseData( moniorPresent );

			return moniorPresent ? transit< CMonitorPresent >() : transit< CWithoutMonitor >();
		}

		return discard_event();

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

	void analyseData( bool & _isMonitorPresent )
	{
		_isMonitorPresent = false;

		BOOST_FOREACH( common::CValidNodeInfo const & validNode, m_uniqueNodes )
		{
			if ( !CTrackerLocalRanking::getInstance()->isInUnidentified( validNode.m_address.ToStringIP() ) )
			{
				if ( validNode.m_role == common::CRole::Monitor )
				{
					_isMonitorPresent = true;
					CTrackerLocalRanking::getInstance()->addMonitor( validNode.m_address.ToStringIP(), common::CNodeStats( validNode.m_key, validNode.m_address.ToStringIP(), common::dimsParams().getDefaultClientPort() ) );
				}
				else if ( validNode.m_role == common::CRole::Tracker )
				{
					CTrackerLocalRanking::getInstance()->addUndeterminedTracker( validNode.m_address.ToStringIP(), common::CNodeStats( validNode.m_key, validNode.m_address.ToStringIP(), common::dimsParams().getDefaultClientPort() ) );
				}
			}
		}
	}


	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CPending >,
	boost::statechart::custom_reaction< common::CClientNetworkInfoEvent >,
	boost::statechart::custom_reaction< common::CErrorEvent >
	> reactions;

	// in  future be  careful with  those
	std::set< common::CValidNodeInfo > m_uniqueNodes;

	std::set< uintptr_t > m_pending;

	int64_t m_lastAskTime;

	std::map< uintptr_t, uint256 > m_nodeToToken;
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
		context< CConnectAction >().setRequest( new CTrackersInfoRequest( new CMediumClassFilter( common::RequestKind::UndeterminedTrackers ) ) );

		m_lastAskTime = GetTime();

		m_trackerAdded = 0;
	}

	boost::statechart::result react( common::CPending const & _pending )
	{
		m_nodeToToken.insert( std::make_pair( _pending.m_networkPtr, _pending.m_token ) );

		m_pending.insert( _pending.m_networkPtr );

		int64_t time = GetTime();
		if ( time - m_lastAskTime < NetworkAskLoopTime )
		{
			if ( !context< CConnectAction >().isRequestReady() )
					context< CConnectAction >().setRequest( new CInfoRequestContinueComplex( m_nodeToToken, new CSpecificMediumFilter( m_pending ) ) );
			return discard_event();
		}
		else
		{
			CClientControl::getInstance()->process_event( CNetworkDiscoveredEvent( m_trackerAdded, 0 ) );// fix  this 0, 0

			context< CConnectAction >().setRequest( 0 );

			return discard_event();
		}
	}

	boost::statechart::result react( common::CTrackerStatsEvent const & _trackerStats )
	{
		common::CNodeStats undeterminedTracker;
		CTrackerLocalRanking::getInstance()->getUndeterminedTracker( _trackerStats.m_ip, undeterminedTracker );

		common::CTrackerStats trackerStats(
			  undeterminedTracker.m_key
			, 0
			, _trackerStats.m_price
			, _trackerStats.m_maxPrice
			, _trackerStats.m_minPrice
			, undeterminedTracker.m_ip
			, undeterminedTracker.m_port
			);

		CTrackerLocalRanking::getInstance()->addTracker( trackerStats );
		m_trackerAdded++;

		CTrackerLocalRanking::getInstance()->removeUndeterminedTracker( _trackerStats.m_ip );

		m_pending.erase( _trackerStats.m_nodeIndicator );

		if ( !m_pending.size() )
		{
			CClientControl::getInstance()->process_event( CNetworkDiscoveredEvent( m_trackerAdded, 0 ) ); // fix  this 0, 0
			context< CConnectAction >().setRequest( 0 );
		}

		return discard_event();

	}

	// continue event

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CPending >,
	boost::statechart::custom_reaction< common::CTrackerStatsEvent >
	> reactions;

	std::set< uintptr_t > m_pending;

	int64_t m_lastAskTime;

	std::map< uintptr_t, uint256 > m_nodeToToken;

	unsigned int m_trackerAdded;
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
	common::CRequest< NodeResponses >* request = m_request;
	m_request = 0;
	return request;
}

bool
CConnectAction::isRequestReady() const
{
	return m_request;
}

void
CConnectAction::setRequest( common::CRequest< NodeResponses >* _request )
{
	m_request = _request;
}

}
