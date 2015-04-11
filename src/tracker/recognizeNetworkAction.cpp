// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "common/manageNetwork.h"
#include "common/actionHandler.h"

#include "tracker/recognizeNetworkAction.h"
#include "tracker/connectNodeAction.h"
#include "tracker/trackerFilters.h"

#include <boost/statechart/state.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/custom_reaction.hpp>

namespace tracker
{

struct CGetDnsInfo : boost::statechart::state< CGetDnsInfo, CRecognizeNetworkAction >
{
	CGetDnsInfo( my_context ctx ) : my_base( ctx )
	{
		// search for  seeder  action
		std::vector<CAddress> vAdd;

		common::CManageNetwork::getInstance()->getIpsFromSeed( vAdd );


context< CRecognizeNetworkAction >().dropRequests();
context< CRecognizeNetworkAction >().addRequests(
			new common::CScheduleActionRequest< common::CTrackerTypes >(
				  new CConnectNodeAction( CAddress( CService("94.156.77.114", 20020) ) )
				, new CMediumClassFilter( common::CMediumKinds::Shedule) ) );
/*		if ( !vAdd.empty() )
		{
			BOOST_FOREACH( CAddress address, vAdd )
			{
				context< CRecognizeNetworkAction >().dropRequests();
				context< CRecognizeNetworkAction >().addRequests(
							new common::CScheduleActionRequest< common::CTrackerTypes >(
								  new CConnectNodeAction( address )
								, new CMediumClassFilter( common::CMediumKinds::Shedule) ) );
			}
		}
		else
		{
			common::CManageNetwork::getInstance()->getSeedIps( vAdd );

			// let know seed about our existence
			BOOST_FOREACH( CAddress address, vAdd )
			{
				context< CRecognizeNetworkAction >().dropRequests();
				context< CRecognizeNetworkAction >().addRequests(
							new common::CScheduleActionRequest< common::CTrackerTypes >(
								  new CConnectNodeAction( address )
								, new CMediumClassFilter( common::CMediumKinds::Shedule) ) );
			}
		}*/
	}

	boost::statechart::result react( common::CNetworkInfoEvent const & _networkInfoEvent )
	{
		//return transit< CDetermineRoleConnecting >();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CNetworkInfoEvent >
	> reactions;

};


CRecognizeNetworkAction::CRecognizeNetworkAction()
{
	initiate();
}

void
CRecognizeNetworkAction::accept( common::CSetResponseVisitor< common::CTrackerTypes > & _visitor )
{
	_visitor.visit( *this );
}

}
