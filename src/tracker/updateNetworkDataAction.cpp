// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/statechart/state.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/custom_reaction.hpp>

#include "common/communicationProtocol.h"
#include "common/requests.h"
#include "common/responses.h"
#include "common/setResponseVisitor.h"

#include "tracker/updateNetworkDataAction.h"
#include "tracker/trackerNodesManager.h"
#include "tracker/filters.h"

namespace tracker
{

struct CUpdateNetworkData : boost::statechart::state< CUpdateNetworkData, CUpdateNetworkDataAction >
{
	CUpdateNetworkData( my_context ctx ) : my_base( ctx )
	{
	}

	boost::statechart::result react( common::CMessageResult const & _messageResult )
	{
		common::CMessage orginalMessage;
		if ( !common::CommunicationProtocol::unwindMessage( _messageResult.m_message, orginalMessage, GetTime(), _messageResult.m_pubKey ) )
			assert( !"service it somehow" );

		context< CUpdateNetworkDataAction >().addRequest(
					new common::CAckRequest(
						  context< CUpdateNetworkDataAction >().getActionKey()
						, _messageResult.m_message.m_header.m_id
						, new CByKeyMediumFilter( _messageResult.m_pubKey ) ) );

		// boring  and  tedious
		common::CRankingFullInfo rankingFullInfo;

		common::convertPayload( orginalMessage, rankingFullInfo );

		std::set<common::CValidNodeInfo> trackers;

		BOOST_FOREACH( common::CTrackerData const & trackerData, rankingFullInfo.m_trackers )
		{
			trackers.insert( common::CValidNodeInfo( trackerData.m_publicKey, trackerData.m_address ) );
		}

		BOOST_FOREACH( common::CValidNodeInfo const & tracker, trackers )
		{
			if ( !CTrackerNodesManager::getInstance()->isInNetwork( tracker.m_publicKey.GetID() ) )
			{
				CTrackerNodesManager::getInstance()->setNetworkTracker( tracker );
			}
		}

		std::set< common::CValidNodeInfo > networkNodes = CTrackerNodesManager::getInstance()->getNetworkTrackers();

		BOOST_FOREACH( common::CValidNodeInfo const & node, networkNodes )
		{
			if ( trackers.find( node ) == trackers.end() )
			{
				CTrackerNodesManager::getInstance()->eraseNetworkTracker( node.m_publicKey.GetID() );
			}
		}

		std::set<common::CValidNodeInfo> monitors;

		BOOST_FOREACH( common::CAllyMonitorData const & allyMonitorData, rankingFullInfo.m_allyMonitors )
		{
			monitors.insert( common::CValidNodeInfo( allyMonitorData.m_publicKey, allyMonitorData.m_address ) );
		}

		BOOST_FOREACH( common::CValidNodeInfo const & monitor, monitors )
		{
			if ( !CTrackerNodesManager::getInstance()->isInNetwork( monitor.m_publicKey.GetID() ) )
			{
				CTrackerNodesManager::getInstance()->setNetworkMonitor( monitor );
			}
		}

		networkNodes = CTrackerNodesManager::getInstance()->getNetworkMonitors();

		BOOST_FOREACH( common::CValidNodeInfo const & node, networkNodes )
		{
			if ( monitors.find( node ) == monitors.end() )
			{
				CTrackerNodesManager::getInstance()->eraseNetworkMonitor( node.m_publicKey.GetID() );
			}
		}

		context< CUpdateNetworkDataAction >().setExit();

		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CMessageResult >
	> reactions;

	std::set< uint160 > m_presentTrackers;
};

CUpdateNetworkDataAction::CUpdateNetworkDataAction( uint256 const & _actionKey )
	: common::CAction( _actionKey )
{
	initiate();
}

void
CUpdateNetworkDataAction::accept( common::CSetResponseVisitor & _visitor )
{
	_visitor.visit( *this );
}

}
