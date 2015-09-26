// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/statechart/transition.hpp>
#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/state.hpp>
#include <boost/statechart/custom_reaction.hpp>

#include <vector>

#include "net.h"
#include "base58.h"

#include "common/actionHandler.h"
#include "common/manageNetwork.h"
#include "common/periodicActionExecutor.h"
#include "common/events.h"

#include "monitor/filters.h"
#include "monitor/connectNodeAction.h"
#include "monitor/updateDataAction.h"
#include "monitor/recognizeNetworkAction.h"
#include "monitor/trackOriginAddressAction.h"
#include "monitor/controller.h"

namespace monitor
{

struct CGetStateEvent : boost::statechart::event< CGetStateEvent >
{
};

struct CConnectedToTrackerEvent : boost::statechart::event< CConnectedToTrackerEvent >
{
	CConnectedToTrackerEvent(){}
};

struct CSynchronizedWithNetworkEvent : boost::statechart::event< CSynchronizedWithNetworkEvent >
{
};

struct CTrackerConnectingEvent : boost::statechart::event< CTrackerConnectingEvent >
{
	CTrackerConnectingEvent(){}
};

struct CConnectWithTrackerRequest : boost::statechart::event< CConnectWithTrackerRequest >
{
	CConnectWithTrackerRequest( std::string const & _trackerAddress ):m_trackerAddress( _trackerAddress ){};

	std::string const m_trackerAddress;
};

struct CMonitorStandAlone;
struct CMonitorConnected;
struct CMonitorSynchronizing;

struct CSynchronizeWithBitcoin : boost::statechart::state< CSynchronizeWithBitcoin, CController >
{
	CSynchronizeWithBitcoin( my_context ctx ) : my_base( ctx )
	{
		common::CActionHandler::getInstance()->executeAction( new CTrackOriginAddressAction() );
	}

	boost::statechart::result
	react( common::CUpdateStatus const & _event )
	{
		std::string status;

		if ( m_nodesNumber < common::dimsParams().getUsedBitcoinNodesNumber() )
		{
			{
				std::ostringstream convert;
				convert << m_nodesNumber;

				status = "Connecting to bitcoin network \n currently there is:  " + convert.str() + "  connections \n";
			}
			{
				std::ostringstream convert;
				convert << common::dimsParams().getUsedBitcoinNodesNumber();

				status += "there is " + convert.str() + " needed";
			}
		}
		else
		{
			status = "Reading bitcoin blockchain for base transactions: \n";

			std::ostringstream convert;
			convert << m_blockLeft;

			status += "left " + convert.str() + " blocks to  be checked..\n";
		}
		context< CController >().setStatusMessage( status );
		return discard_event();
	}

	boost::statechart::result
	react( common::CBitcoinNetworkConnection const & _event )
	{
		m_nodesNumber = _event.m_nodesNumber;
		return discard_event();
	}

	boost::statechart::result
	react( common::CSetScanBitcoinChainProgress const & _event )
	{
		m_blockLeft = _event.m_blockLeft;
		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CUpdateStatus >,
	boost::statechart::custom_reaction< common::CBitcoinNetworkConnection >,
	boost::statechart::custom_reaction< common::CSetScanBitcoinChainProgress >,
	boost::statechart::transition< common::CInitialSynchronizationDoneEvent, CMonitorStandAlone > > reactions;

	int m_blockLeft;
	unsigned int m_nodesNumber;
};

struct CMonitorStandAlone : boost::statechart::state< CMonitorStandAlone, CController >
{
	CMonitorStandAlone( my_context ctx ) : my_base( ctx )
	{
		context< CController >().setStatusMessage( "detecting existing network" );
		common::CActionHandler::getInstance()->executeAction( new CRecognizeNetworkAction() );
	}

	boost::statechart::result
	react( common::CNetworkRecognizedData const & _event )
	{
		std::string status;

		status = "\nDetected following trackers \n";

		BOOST_FOREACH( common::CValidNodeInfo const & nodeInfo, _event.m_trackersInfo )
		{
			CNodeAddress tracker;
			tracker.Set( nodeInfo.m_key.GetID(), common::NodePrefix::Tracker );
			status = "key " + tracker.ToString() + "ip " + nodeInfo.m_address.ToString() + "\n";
		}

		status = "\nDetected following monitors \n";
		BOOST_FOREACH( common::CValidNodeInfo const & nodeInfo, _event.m_monitorsInfo )
		{
			CNodeAddress monitor;
			monitor.Set( nodeInfo.m_key.GetID(), common::NodePrefix::Monitor );
			status = "key " + monitor.ToString() + "ip " + nodeInfo.m_address.ToString() + "\n";
		}

		context< CController >().setStatusMessage( status );
		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CNetworkRecognizedData > > reactions;
};

// not right, since  first connection will trigger synchronization( apply  wait  time to  allow  other connections to appear ??)
// this is outside action handler so I can't deffer this in "normal way"
// is  this irrelevant ???????, when monitors will came this  will change anyway ??????

struct CMonitorSynchronizing : boost::statechart::state< CMonitorSynchronizing, CController >
{

	CMonitorSynchronizing( my_context ctx ) : my_base( ctx )
	{
	/*	CSynchronizationAction * synchronizationAction = new CSynchronizationAction();

		common::CActionHandler::getInstance()->executeAction( synchronizationAction );
		synchronizationAction->process_event( CSwitchToSynchronizing() );
*/
	}

	typedef boost::mpl::list<
	boost::statechart::transition< CSynchronizedWithNetworkEvent, CMonitorConnected > > reactions;
};

struct CMonitorConnected : boost::statechart::state< CMonitorConnected, CController >
{
	CMonitorConnected( my_context ctx ) : my_base( ctx )
	{
	//CController::getInstance()->setConnected( true );
	}

	boost::statechart::result react( CGetStateEvent const & _event )
	{
		return discard_event();
	}

	typedef boost::mpl::list<
	  boost::statechart::custom_reaction< CGetStateEvent > > reactions;
};

CController * CController::ms_instance = NULL;

CController*
CController::getInstance()
{
	if ( !ms_instance )
	{
		ms_instance = new CController();
	};
	return ms_instance;
}

CController::CController()
	: m_enterancePrice(2000)
	, m_price( 3000 )
	, m_period( 3600*4 )
	, m_admitted( false )
{
	initiate();
}

}
