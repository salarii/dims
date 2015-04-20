// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "common/manageNetwork.h"
#include "common/actionHandler.h"

#include "monitor/recognizeNetworkAction.h"
#include "monitor/connectNodeAction.h"
#include "monitor/filters.h"

#include <boost/statechart/state.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/custom_reaction.hpp>

namespace monitor
{

struct CGetDnsInfo : boost::statechart::state< CGetDnsInfo, CRecognizeNetworkAction >
{
	CGetDnsInfo( my_context ctx ) : my_base( ctx )
	{
		// search for  seeder  action
		std::vector<CAddress> vAdd;

		common::CManageNetwork::getInstance()->getIpsFromSeed( vAdd );

		if ( !vAdd.empty() )
		{
			BOOST_FOREACH( CAddress address, vAdd )
			{
				context< CRecognizeNetworkAction >().dropRequests();
				context< CRecognizeNetworkAction >().addRequests(
							new common::CScheduleActionRequest< common::CMonitorTypes >(
								new CConnectNodeAction( address )
								, new CMediumClassFilter( common::CMediumKinds::Schedule) ) );
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
							new common::CScheduleActionRequest< common::CMonitorTypes >(
								new CConnectNodeAction( address )
								, new CMediumClassFilter( common::CMediumKinds::Schedule) ) );
			}
		}
	}

	boost::statechart::result react( common::CNetworkInfoEvent const & _networkInfoEvent )
	{
		//return transit< CDetermineRoleConnecting >();
		return discard_event();
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
CRecognizeNetworkAction::accept( common::CSetResponseVisitor< common::CMonitorTypes > & _visitor )
{
	_visitor.visit( *this );
}

}
