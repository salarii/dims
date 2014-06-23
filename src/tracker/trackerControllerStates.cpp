// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "trackerControllerStates.h"

namespace tracker
{

CStandAlone::CStandAlone( my_context ctx ) : my_base( ctx )
{
	// search for  seeder  action

}

//simple_state::triggering_event()

boost::statechart::result
CStandAlone::react( CGetStateEvent const & _event )
{
	return discard_event();
}

CConnected::CConnected( my_context ctx ) : my_base( ctx )
{

}


}
