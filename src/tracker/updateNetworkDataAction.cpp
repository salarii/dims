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
#include "common/actionHandler.h"

#include "tracker/updateNetworkDataAction.h"
#include "tracker/trackerNodesManager.h"
#include "tracker/filters.h"
#include "tracker/connectNodeAction.h"

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

		BOOST_FOREACH( common::CAllyTrackerData const & trackerData, rankingFullInfo.m_allyTrackers )
		{
			if ( rankingFullInfo.m_synchronizedTrackers.find( trackerData.m_publicKey.GetID() ) != rankingFullInfo.m_synchronizedTrackers.end() )
				trackers.insert( common::CValidNodeInfo( trackerData.m_publicKey, trackerData.m_address ) );
		}

		BOOST_FOREACH( common::CTrackerData const & trackerData, rankingFullInfo.m_trackers )
		{
			if ( rankingFullInfo.m_synchronizedTrackers.find( trackerData.m_publicKey.GetID() ) != rankingFullInfo.m_synchronizedTrackers.end() )
				trackers.insert( common::CValidNodeInfo( trackerData.m_publicKey, trackerData.m_address ) );
		}

		BOOST_FOREACH( common::CValidNodeInfo const & tracker, trackers )
		{
			if ( !CTrackerNodesManager::getInstance()->isInNetwork( tracker.m_publicKey.GetID() ) )
			{
				CTrackerNodesManager::getInstance()->setNetworkTracker( tracker );
				common::CActionHandler ::getInstance()->executeAction( new CConnectNodeAction( tracker.m_address ) );
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

		CAddress address;
		if ( !CTrackerNodesManager::getInstance()->getAddresFromKey( _messageResult.m_pubKey.GetID(), address ) )
			assert(!"problem");

		monitors.insert( common::CValidNodeInfo( _messageResult.m_pubKey, address ) );// do I need  this??

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
			if ( monitors.find( node ) == monitors.end() && _messageResult.m_pubKey != node.m_publicKey )
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
	LogPrintf("update network data action: %p \n", this );

	initiate();
}

void
CUpdateNetworkDataAction::accept( common::CSetResponseVisitor & _visitor )
{
	_visitor.visit( *this );
}

}
