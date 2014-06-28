// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "connectNodeAction.h"
#include "connectToTrackerRequest.h"
#include "common/setResponseVisitor.h"
#include "common/commonEvents.h"
#include "common/authenticationProvider.h"
#include "common/mediumRequests.h"
#include "trackerNodesManager.h"

#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/state.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/custom_reaction.hpp>

namespace tracker
{


struct CUnconnected; struct CBothUnidentifiedConnected;


struct CUninitiated : boost::statechart::simple_state< CUninitiated, CConnectNodeAction >
{
	typedef boost::mpl::list<
	boost::statechart::transition< common::CSwitchToConnectingEvent, CUnconnected >,
	boost::statechart::transition< common::CSwitchToConnectedEvent, CBothUnidentifiedConnected >
	> reactions;

};


struct CIdentified : boost::statechart::state< CIdentified, CConnectNodeAction >
{
	CIdentified( my_context ctx ) : my_base( ctx )
	{
		// for  now we  finish here
		context< CConnectNodeAction >().setRequest( 0 );
	}
};


template < class Parent >
void
createIdentifyResponse( Parent & parent )
{
	uint256 hash = Hash( &parent.getPayload().front(), &parent.getPayload().back() );

	std::vector< unsigned char > signedHash;
	common::CAuthenticationProvider::getInstance()->sign( hash, signedHash );

	parent.setRequest( new common::CIdentifyResponse<TrackerResponses>( parent.getMediumKind(), signedHash, common::CAuthenticationProvider::getInstance()->getMyKeyId(), parent.getPayload(), parent.getActionKey() ) );
}

struct CPairIdentifiedConnecting : boost::statechart::state< CPairIdentifiedConnecting, CConnectNodeAction >
{
	CPairIdentifiedConnecting( my_context ctx ) : my_base( ctx )
	{
		common::CIntroduceEvent const* requestedEvent = dynamic_cast< common::CIntroduceEvent const* >( simple_state::triggering_event() );

		createIdentifyResponse( context< CConnectNodeAction >() );
	}

	typedef boost::mpl::list<
	boost::statechart::transition< common::CContinueEvent, CIdentified >// kind of using side effect is this ok??
	> reactions;

};

struct CPairIdentifiedConnected : boost::statechart::state< CPairIdentifiedConnected, CConnectNodeAction >
{
	CPairIdentifiedConnected( my_context ctx ) : my_base( ctx )
	{
		context< CConnectNodeAction >().setRequest( 0 );
	}
};


struct CBothUnidentifiedConnecting : boost::statechart::state< CBothUnidentifiedConnecting, CConnectNodeAction >
{
	CBothUnidentifiedConnecting( my_context ctx ) : my_base( ctx )
	{

		common::CNodeConnectedEvent const* connectedEvent = dynamic_cast< common::CNodeConnectedEvent const* >( simple_state::triggering_event() );
		context< CConnectNodeAction >().setMediumKind( convertToInt( connectedEvent->m_node ) );
		// looks funny that  I set it in this  state, but let  it  be
		CTrackerNodesManager::getInstance()->addNode( connectedEvent->m_node );

		context< CConnectNodeAction >().setRequest( new common::CIdentifyRequest<TrackerResponses>( convertToInt( connectedEvent->m_node ), context< CConnectNodeAction >().getPayload(), context< CConnectNodeAction >().getActionKey() ) );

	}

	boost::statechart::result react( const common::CContinueEvent & _continueEvent )
	{
		context< CConnectNodeAction >().setRequest( new common::CContinueReqest<TrackerResponses>( _continueEvent.m_keyId, context< CConnectNodeAction >().getMediumKind() ) );
	}

	typedef boost::mpl::list<
	boost::statechart::transition< common::CIntroduceEvent, CPairIdentifiedConnecting >,
	boost::statechart::custom_reaction< common::CContinueEvent >
	> reactions;
};

struct CBothUnidentifiedConnected : boost::statechart::state< CBothUnidentifiedConnected, CConnectNodeAction >
{
	CBothUnidentifiedConnected( my_context ctx ) : my_base( ctx )
	{
		common::CIntroduceEvent const* requestedEvent = dynamic_cast< common::CIntroduceEvent const* >( simple_state::triggering_event() );

		createIdentifyResponse( context< CConnectNodeAction >() );

	}

	boost::statechart::result react( const common::CContinueEvent & _continueEvent )
	{
		context< CConnectNodeAction >().setRequest( new common::CContinueReqest<TrackerResponses>( _continueEvent.m_keyId, context< CConnectNodeAction >().getMediumKind() ) );
	}

	typedef boost::mpl::list<
	boost::statechart::transition< common::CIntroduceEvent, CPairIdentifiedConnected >,
	boost::statechart::custom_reaction< common::CContinueEvent >
	> reactions;
};

struct CUnconnected : boost::statechart::state< CUnconnected, CConnectNodeAction >
{
	CUnconnected( my_context ctx ) : my_base( ctx )
	{
		context< CConnectNodeAction >().setRequest(
				  new CConnectToTrackerRequest( context< CConnectNodeAction >().getAddress(), context< CConnectNodeAction >().getServiceAddress() ) );

	}

	typedef boost::mpl::list<
	boost::statechart::transition< common::CNodeConnectedEvent, CBothUnidentifiedConnecting >
	> reactions;

};

struct CSynchronizing : boost::statechart::simple_state< CSynchronizing, CConnectNodeAction >
{

};

CConnectNodeAction::CConnectNodeAction( std::vector< unsigned char > const & _payload, unsigned int _mediumKind )
: m_payload( _payload )
, m_request( 0 )
, m_passive( true )
, m_mediumKind( _mediumKind )
{
	initiate();
	process_event( common::CSwitchToConnectedEvent() );
}

CConnectNodeAction::CConnectNodeAction( CAddress const & _addrConnect )
	: m_request( 0 )
	, m_passive( false )
	, m_addrConnect( _addrConnect )
{
	for ( unsigned int i = 0; i < ms_randomPayloadLenght; i++ )
	{
		m_payload.push_back( insecure_rand() % 256 );
	}
	initiate();
	process_event( common::CSwitchToConnectingEvent() );
}


CConnectNodeAction::CConnectNodeAction( std::string const & _nodeAddress )
	: m_nodeAddress( _nodeAddress )
	, m_request( 0 )
	, m_passive( false )
{
	for ( unsigned int i = 0; i < ms_randomPayloadLenght; i++ )
	{
		m_payload.push_back( insecure_rand() % 256 );
	}
	initiate();
	process_event( common::CSwitchToConnectingEvent() );
}

common::CRequest< TrackerResponses >*
CConnectNodeAction::execute()
{
	return m_request;
}

void
CConnectNodeAction::accept( common::CSetResponseVisitor< TrackerResponses > & _visitor )
{
	_visitor.visit( *this );
}

void
CConnectNodeAction::setRequest( common::CRequest< TrackerResponses >* _request )
{
	m_request = _request;
}

std::string
CConnectNodeAction::getAddress() const
{
	return m_nodeAddress;
}

CAddress
CConnectNodeAction::getServiceAddress() const
{
	return m_addrConnect;
}

std::vector< unsigned char >
CConnectNodeAction::getPayload() const
{
	return m_payload;
}

unsigned int
CConnectNodeAction::getMediumKind() const
{
	return m_mediumKind;
}

void
CConnectNodeAction::setMediumKind( unsigned int _mediumKind )
{
	m_mediumKind = _mediumKind;
}

CConnectNodeAction::~CConnectNodeAction()
{
	common::CNodesManager< TrackerResponses >::getInstance()->unregisterAction( m_mediumKind, m_actionKey );
}

}
