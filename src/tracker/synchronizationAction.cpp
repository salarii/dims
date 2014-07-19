// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


#include "common/setResponseVisitor.h"
#include "common/mediumRequests.h"
#include "common/commonEvents.h"

#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/state.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/custom_reaction.hpp>

#include "trackerMediumsKinds.h"

#include "synchronizationAction.h"

#define CONFIRM_LIMIT 6

namespace tracker
{

struct CUninitiated : boost::statechart::simple_state< CUninitiated, CSynchronizationAction >
{
};

CSynchronizationAction::CSynchronizationAction()
{
	initiate();
}



common::CRequest< TrackerResponses >*
CSynchronizationAction::execute()
{
	return m_request;
}

void
CSynchronizationAction::accept( common::CSetResponseVisitor< TrackerResponses > & _visitor )
{
	_visitor.visit( *this );
}

void
CSynchronizationAction::setRequest( common::CRequest< TrackerResponses >* _request )
{
	m_request = _request;
}

void
CSynchronizationAction::clear()
{

}

}

