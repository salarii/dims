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
#include <boost/statechart/custom_reaction.hpp>
#include "common/authenticationProvider.h"
#include "identifyRequest.h"
#include "nodesManager.h"
#include "continueRequest.h"

namespace tracker
{


struct CUnconnected; struct CBothUnidentifiedConnected;

struct CUninitiated : boost::statechart::simple_state< CUninitiated, CConnectTrackerAction >
{
	typedef boost::mpl::list<
	boost::statechart::transition< CSwitchToConnectingEvent, CUnconnected >,
	boost::statechart::transition< CSwitchToConnectedEvent, CBothUnidentifiedConnected >
	> reactions;

};

struct CIdentified : boost::statechart::state< CIdentified, CConnectTrackerAction >
{
	CIdentified( my_context ctx ) : my_base( ctx )
	{
		// for  now we  finish here
		context< CConnectTrackerAction >().setRequest( 0 );
	}
};


template < class Parent >
void
createIdentifyResponse( Parent & parent )
{
	uint256 hash = Hash( &parent.getPayload().front(), &parent.getPayload().back() );

	std::vector< unsigned char > signedHash;
	common::CAuthenticationProvider::getInstance()->sign( hash, signedHash );

	parent.setRequest( new CIdentifyResponse( parent.getMediumKind(), signedHash, common::CAuthenticationProvider::getInstance()->getMyKeyId(), parent.getPayload() ) );
}

struct CPairIdentifiedConnecting : boost::statechart::state< CPairIdentifiedConnecting, CConnectTrackerAction >
{
	CPairIdentifiedConnecting( my_context ctx ) : my_base( ctx )
	{
		CIntroduceEvent const* requestedEvent = dynamic_cast< CIntroduceEvent const* >( simple_state::triggering_event() );

		createIdentifyResponse( context< CConnectTrackerAction >() );
	}

	typedef boost::mpl::list<
	boost::statechart::transition< CContinueEvent, CIdentified >// kind of using side effect is this ok??
	> reactions;

};

struct CPairIdentifiedConnected : boost::statechart::state< CPairIdentifiedConnected, CConnectTrackerAction >
{
	CPairIdentifiedConnected( my_context ctx ) : my_base( ctx )
	{
		context< CConnectTrackerAction >().setRequest( 0 );
	}
};


struct CBothUnidentifiedConnecting : boost::statechart::state< CBothUnidentifiedConnecting, CConnectTrackerAction >
{
	CBothUnidentifiedConnecting( my_context ctx ) : my_base( ctx )
	{

		CNodeConnectedEvent const* connectedEvent = dynamic_cast< CNodeConnectedEvent const* >( simple_state::triggering_event() );
		context< CConnectTrackerAction >().setMediumKind( convertToInt( connectedEvent->m_node ) );
		// looks funny that  I set it in this  state, but let  it  be
		CNodesManager::getInstance()->addNode( connectedEvent->m_node );
		context< CConnectTrackerAction >().setRequest( new CIdentifyRequest( convertToInt( connectedEvent->m_node ), context< CConnectTrackerAction >().getPayload() ) );

	}

	boost::statechart::result react( const CContinueEvent & _continueEvent )
	{
		context< CConnectTrackerAction >().setRequest( new CContinueReqest( _continueEvent.m_keyId, context< CConnectTrackerAction >().getMediumKind() ) );
	}

	typedef boost::mpl::list<
	boost::statechart::transition< CIntroduceEvent, CPairIdentifiedConnecting >,
	boost::statechart::custom_reaction< CContinueEvent >
	> reactions;
};

struct CBothUnidentifiedConnected : boost::statechart::state< CBothUnidentifiedConnected, CConnectTrackerAction >
{
	CBothUnidentifiedConnected( my_context ctx ) : my_base( ctx )
	{
		CIntroduceEvent const* requestedEvent = dynamic_cast< CIntroduceEvent const* >( simple_state::triggering_event() );

		createIdentifyResponse( context< CConnectTrackerAction >() );

	}

	boost::statechart::result react( const CContinueEvent & _continueEvent )
	{
		context< CConnectTrackerAction >().setRequest( new CContinueReqest( _continueEvent.m_keyId, context< CConnectTrackerAction >().getMediumKind() ) );
	}

	typedef boost::mpl::list<
	boost::statechart::transition< CIntroduceEvent, CPairIdentifiedConnected >,
	boost::statechart::custom_reaction< CContinueEvent >
	> reactions;
};

struct CUnconnected : boost::statechart::state< CUnconnected, CConnectTrackerAction >
{
	CUnconnected( my_context ctx ) : my_base( ctx )
	{
		context< CConnectTrackerAction >().setRequest(
				  new CConnectToTrackerRequest( context< CConnectTrackerAction >().getAddress() ) );

	}

	typedef boost::mpl::list<
	boost::statechart::transition< CNodeConnectedEvent, CBothUnidentifiedConnecting >
	> reactions;

};

struct CSynchronizing : boost::statechart::simple_state< CSynchronizing, CConnectTrackerAction >
{

};

CConnectTrackerAction::CConnectTrackerAction( std::vector< unsigned char > const & _payload, unsigned int _mediumKind )
: m_payload( _payload )
, m_request( 0 )
, m_passive( true )
, m_mediumKind( _mediumKind )
{
	initiate();
	process_event( CSwitchToConnectedEvent() );
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
	process_event( CSwitchToConnectingEvent() );
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

unsigned int
CConnectTrackerAction::getMediumKind() const
{
	return m_mediumKind;
}

void
CConnectTrackerAction::setMediumKind( unsigned int _mediumKind )
{
	m_mediumKind = _mediumKind;
}

}
