// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "boost/foreach.hpp"
#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/state.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/custom_reaction.hpp>

#include "protocol.h"
#include "base58.h"

#include "common/manageNetwork.h"
#include "common/actionHandler.h"

#include "tracker/connectNodeAction.h"
#include "tracker/synchronizationAction.h"
#include "tracker/events.h"
#include "tracker/controller.h"
#include "tracker/trackOriginAddressAction.h"
#include "tracker/recognizeNetworkAction.h"
#include "tracker/controllerEvents.h"

namespace tracker
{

struct CStandAlone;
struct CConnected;

CController * CController::ms_instance = NULL;

struct CInitialSynchronization : boost::statechart::simple_state< CInitialSynchronization, CController >
{
	CInitialSynchronization()
		:m_blockLeft( -1 )
	{
		common::CActionHandler::getInstance()->executeAction( new tracker::CTrackOriginAddressAction );
	}

	boost::statechart::result react( common::CSetScanBitcoinChainProgress const & _event )
	{
		m_blockLeft = _event.m_blockLeft;
		return discard_event();
	}

	boost::statechart::result react( common::CUpdateStatus const & _event )
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

	boost::statechart::result react( common::CBitcoinNetworkConnection const & _event )
	{
		m_nodesNumber = _event.m_nodesNumber;
		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CUpdateStatus >,
	boost::statechart::custom_reaction< common::CBitcoinNetworkConnection >,
	boost::statechart::custom_reaction< common::CSetScanBitcoinChainProgress >,
	boost::statechart::transition< common::CInitialSynchronizationDoneEvent, CStandAlone > > reactions;

	int m_blockLeft;
	unsigned int m_nodesNumber;
};


struct CConnected;
struct CLeading;
struct CSynchronizing;

struct CStandAlone : boost::statechart::state< CStandAlone, CController >
{
	CStandAlone( my_context ctx ) : my_base( ctx )
	{
		context< CController >().setStatusMessage( "detecting existing network" );
		common::CActionHandler::getInstance()->executeAction( new CRecognizeNetworkAction() );
	}

	boost::statechart::result react( common::CNetworkRecognizedData const & _event )
	{
		std::string status;

		context< CController >().setNetworkData( _event );

		if ( _event.m_trackersInfo.empty() )
		{
			status = "\nNo trackers detected\n";
		}
		else
		{
			status = "\nDetected following trackers \n";

			BOOST_FOREACH( common::CValidNodeInfo const & nodeInfo, _event.m_trackersInfo )
			{
				CNodeAddress tracker;
				tracker.Set( nodeInfo.m_key.GetID(), common::NodePrefix::Tracker );
				status += "key " + tracker.ToString() + "ip " + nodeInfo.m_address.ToString() + "\n";
			}
		}

		if ( _event.m_monitorsInfo.empty() )
		{
			status += "\nNo monitors detected\n";
		}
		else
		{
			status += "\nDetected following monitors \n";
			BOOST_FOREACH( common::CValidNodeInfo const & nodeInfo, _event.m_monitorsInfo )
			{
				CNodeAddress monitor;
				monitor.Set( nodeInfo.m_key.GetID(), common::NodePrefix::Monitor );

				status += "key: " + monitor.ToString() + " ip: " + nodeInfo.m_address.ToString() + "\n";
			}
		}
		context< CController >().setStatusMessage( status );
		return discard_event();
	}

	boost::statechart::result react( common::CRegistrationData const & _event )
	{
		std::string status = context< CController >().getStatusMessage();

		if ( !_event.m_registrationTime )
		{
			status += "\n not registered \n";

			status += "\n in order to perform any action within network you need to be registered";

		}
		else
		{
			CNodeAddress monitor;
			monitor.Set( _event.m_key.GetID(), common::NodePrefix::Monitor );
			status += "\n registered in monitor: " + monitor.ToString() +"\n";

			int64_t timeLeft = _event.m_registrationTime + _event.m_period - GetTime();

			std::ostringstream convert;

			convert << timeLeft;

			status += "\nleft: " + convert.str() + " second of registration time \n";

			status += "\nyou may connect to network now,";
			status += "\nthis will cause synchronization";
			status += "\nand start performing normal operation within network";
		}

		context< CController >().setStatusMessage( status );

		context< CController >().setRegistrationData( _event );

		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CNetworkRecognizedData >,
	boost::statechart::custom_reaction< common::CRegistrationData >,
	boost::statechart::transition< CMonitorAcceptEvent, CConnected > > reactions;
};

struct CConnected : boost::statechart::state< CConnected, CController >
{
	CConnected( my_context ctx ) : my_base( ctx )
	{
		CMonitorAcceptEvent const* monitorAcceptEvent =
				dynamic_cast< CMonitorAcceptEvent const* >( simple_state::triggering_event() );

		std::string status;

		status += "\n Operating in network \n";

		CNodeAddress monitor;
		monitor.Set( monitorAcceptEvent->m_monitorKey.GetID(), common::NodePrefix::Monitor );
		status += "\n registered in monitor: " + monitor.ToString() +"\n";

		context< CController >().setStatusMessage( status );

		CController::getInstance()->setConnected( true );
	}

	boost::statechart::result react( CGetStateEvent const & _event )
	{
		return discard_event();
	}

	typedef boost::mpl::list<
	  boost::statechart::custom_reaction< CGetStateEvent > > reactions;
};

CController*
CController::getInstance()
{
	if ( !ms_instance )
	{
		ms_instance = new CController();
	};
	return ms_instance;
}

unsigned int
CController::getPrice() const
{
	return m_price;
}

void
CController::setPrice( unsigned int _price )
{
	m_price = _price;
}

bool
CController::isConnected() const
{
	return m_connected;
}

bool
CController::setConnected( bool _connected )
{
	m_connected = _connected;
	return true;
}

CController::CController()
	: m_price(1000)
	, m_connected( false )
	, m_deviation(0.001)
{
	initiate();
}

}
