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
#include "authenticationProvider.h"
#include "identifyRequest.h"

namespace tracker
{
struct CUnidentifiedResp;struct CUnidentifiedReq;

struct CConnecting : boost::statechart::state< CConnecting, CConnectTrackerAction >
{
	CConnecting( my_context ctx ) : my_base( ctx )
	{
		context< CConnectTrackerAction >().setRequest(
				  new CConnectToTrackerRequest( context< CConnectTrackerAction >().getAddress() ) );

	}

	typedef boost::mpl::list<
	boost::statechart::transition< CIntroduceEvent, CUnidentifiedResp >,
	boost::statechart::transition< CNodeConnectedEvent, CUnidentifiedReq >
	> reactions;

};

struct CUnidentifiedResp : boost::statechart::state< CUnidentifiedResp, CConnectTrackerAction >
{
	CUnidentifiedResp( my_context ctx ) : my_base( ctx )
	{
		CIntroduceEvent const* requestedEvent = dynamic_cast< CIntroduceEvent const* >( simple_state::triggering_event() );

		uint256 hash = Hash( &context< CConnectTrackerAction >().getPayload().front(), &context< CConnectTrackerAction >().getPayload().back() );

		std::vector< unsigned char > signedHash;
		CAuthenticationProvider::getInstance()->sign( hash, signedHash );

		context< CConnectTrackerAction >().setRequest( new CIdentifyResponse( requestedEvent->m_node, signedHash, CAuthenticationProvider::getInstance()->getMyKeyId() ) );
	}
};

struct CUnidentifiedReq : boost::statechart::state< CUnidentifiedReq, CConnectTrackerAction >
{
	CUnidentifiedReq( my_context ctx ) : my_base( ctx )
	{
		CNodeConnectedEvent const* connectedEvent = dynamic_cast< CNodeConnectedEvent const* >( simple_state::triggering_event() );

		context< CConnectTrackerAction >().setRequest( new CIdentifyRequest( connectedEvent->m_node, context< CConnectTrackerAction >().getPayload() ) );
	}

	typedef boost::mpl::list<
	boost::statechart::transition< CIntroduceEvent, CUnidentifiedResp >
	> reactions;

};

struct CIdentified : boost::statechart::simple_state< CIdentified, CConnectTrackerAction >
{

};

struct CSynchronizing : boost::statechart::simple_state< CSynchronizing, CConnectTrackerAction >
{

};

CConnectTrackerAction::CConnectTrackerAction( std::vector< unsigned char > const & _payload )
: m_payload( _payload )
, m_request( 0 )
, m_passive( true )
{
	initiate();
}

CConnectTrackerAction::CConnectTrackerAction( std::string const & _trackerAddress )
	: m_trackerAddress( _trackerAddress )
	, m_request( 0 )
	, m_passive( false )
{
	for ( unsigned int i = 0; i < ms_randomPayloadLenght; i++ )
	{
		m_payload.push_back( insecure_rand() % 256 );
	}
	initiate();
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

std::vector< unsigned char >
CConnectTrackerAction::getPayload() const
{
	return m_payload;
}

}
