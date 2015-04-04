// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "common/manageNetwork.h"
#include "common/actionHandler.h"

#include "tracker/recognizeNetworkAction.h"
#include "tracker/connectNodeAction.h"

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

		if ( !vAdd.empty() )
		{
			BOOST_FOREACH( CAddress address, vAdd )
			{
				common::CActionHandler< common::CTrackerTypes >::getInstance()->executeAction( new CConnectNodeAction( address ) );
			}
		}
		else
		{
			common::CManageNetwork::getInstance()->getSeedIps( vAdd );

			// let know seed about our existence
			BOOST_FOREACH( CAddress address, vAdd )
			{
				common::CActionHandler< common::CTrackerTypes >::getInstance()->executeAction( new CConnectNodeAction( address ) );
			}
		}
	}

	boost::statechart::result react()
	{
		//return transit< CDetermineRoleConnecting >();
	}

	typedef boost::mpl::list<
//	boost::statechart::custom_reaction< common::CIdentificationResult >
	> reactions;

};


CRecognizeNetworkAction::CRecognizeNetworkAction()
{

}

void
CRecognizeNetworkAction::accept( common::CSetResponseVisitor< common::CTrackerTypes > & _visitor )
{

}

}
