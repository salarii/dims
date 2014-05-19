// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "connectTrackerAction.h"
#include "connectToTrackerRequest.h"
#include "common/setResponseVisitor.h"
#include "connectTrackerActionEvents.h"
#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/state.hpp>
#include <boost/statechart/transition.hpp>

namespace tracker
{
struct CUnidentified;

struct CConnecting : boost::statechart::state< CConnecting, CConnectTrackerAction >
{
	CConnecting( my_context ctx ) : my_base( ctx )
	{
		context< CConnectTrackerAction >().setRequest( new CConnectToTrackerRequest( context< CConnectTrackerAction >().getAddress() ) );
	}

	typedef boost::statechart::transition< CNodeConnectedEvent, CUnidentified > reactions;
};

struct CUnidentified : boost::statechart::simple_state< CUnidentified, CConnectTrackerAction >
{
	CUnidentified()
	{

	}
};

struct CIdentified : boost::statechart::simple_state< CIdentified, CConnectTrackerAction >
{

};

struct CSynchronizing : boost::statechart::simple_state< CSynchronizing, CConnectTrackerAction >
{

};

CConnectTrackerAction::CConnectTrackerAction( std::string const & _trackerAddress )
	: m_trackerAddress( _trackerAddress )
	, m_request( 0 )
{
}

common::CRequest< TrackerResponses >*
CConnectTrackerAction::execute()
{
	return m_request;
}

void
CConnectTrackerAction::accept( common::CSetResponseVisitor< TrackerResponses > & _visitor )
{
	_visitor.visit( *this );
}

void
CConnectTrackerAction::setRequest( common::CRequest< TrackerResponses >* _request )
{
	m_request = _request;
}

std::string
CConnectTrackerAction::getAddress() const
{
	return m_trackerAddress;
}

}
