// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "trackOriginAddressAction.h"
#include "common/setResponseVisitor.h"
#include "common/mediumRequests.h"
#include "common/commonEvents.h"

#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/state.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/custom_reaction.hpp>

#include "trackerMediumsKinds.h"

namespace tracker
{


/*
store last  tracked  block  number
track blocks  between  last  checked  and present
use  at  least  three node  and  compare  results, in case of inconsistency of data  replace  wrong node  by  other correct  one


 what I need is bicoin node  medium

*/
struct CUninitiated : boost::statechart::state< CUninitiated, CTrackOriginAddressAction >
{
	CUninitiated( my_context ctx ) : my_base( ctx )
	{
		if ( vNodes.size() >= 3 )
			;// could proceed  with origin address scanning

		context< CTrackOriginAddressAction >().setRequest( new common::CContinueReqest<TrackerResponses>( 0, CTrackerMediumsKinds::Internal ) );
	}

	boost::statechart::result react( const common::CContinueEvent & _continueEvent )
	{
		context< CTrackOriginAddressAction >().setRequest( new common::CContinueReqest<TrackerResponses>( 0, CTrackerMediumsKinds::Internal ) );
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CContinueEvent >
	> reactions;

};

CTrackOriginAddressAction::CTrackOriginAddressAction()
{
	initiate();
}



common::CRequest< TrackerResponses >*
CTrackOriginAddressAction::execute()
{
	return m_request;
}

void
CTrackOriginAddressAction::accept( common::CSetResponseVisitor< TrackerResponses > & _visitor )
{
	_visitor.visit( *this );
}

void
CTrackOriginAddressAction::setRequest( common::CRequest< TrackerResponses >* _request )
{
	m_request = _request;
}

}




