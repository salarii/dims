// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "acceptNodeAction.h"
#include "common/setResponseVisitor.h"
#include "common/commonEvents.h"
#include "common/authenticationProvider.h"
#include "common/mediumRequests.h"
#include "seedNodesManager.h"

#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/state.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/custom_reaction.hpp>

#include "acceptNodeAction.h"

#include "seedDb.h"

namespace seed
{
extern CAddrDb db;

struct CUnconnected; struct CBothUnidentifiedConnected;


struct CUninitiated : boost::statechart::simple_state< CUninitiated, CAcceptNodeAction >
{
	typedef boost::mpl::list<
	boost::statechart::transition< common::CSwitchToConnectingEvent, CUnconnected >,
	boost::statechart::transition< common::CSwitchToConnectedEvent, CBothUnidentifiedConnected >
	> reactions;

};


struct CIdentified : boost::statechart::state< CIdentified, CAcceptNodeAction >
{
	CIdentified( my_context ctx ) : my_base( ctx )
	{
		context< CAcceptNodeAction >().setValid( true );
		context< CAcceptNodeAction >().setRequest( 0 );
	}
};


template < class Parent >
void
createIdentifyResponse( Parent & parent )
{
	uint256 hash = Hash( &parent.getPayload().front(), &parent.getPayload().back() );

	std::vector< unsigned char > signedHash;
	common::CAuthenticationProvider::getInstance()->sign( hash, signedHash );

	parent.setRequest( new common::CIdentifyResponse<SeedResponses>( parent.getMediumKind(), signedHash, common::CAuthenticationProvider::getInstance()->getMyKeyId(), parent.getPayload() ) );
}

struct CPairIdentifiedConnecting : boost::statechart::state< CPairIdentifiedConnecting, CAcceptNodeAction >
{
	CPairIdentifiedConnecting( my_context ctx ) : my_base( ctx )
	{
		common::CIntroduceEvent const* requestedEvent = dynamic_cast< common::CIntroduceEvent const* >( simple_state::triggering_event() );

		createIdentifyResponse( context< CAcceptNodeAction >() );
	}

	typedef boost::mpl::list<
	boost::statechart::transition< common::CContinueEvent, CIdentified >// kind of using side effect is this ok??
	> reactions;

};

struct CPairIdentifiedConnected : boost::statechart::state< CPairIdentifiedConnected, CAcceptNodeAction >
{
	CPairIdentifiedConnected( my_context ctx ) : my_base( ctx )
	{
		common::CIntroduceEvent const* requestedEvent = dynamic_cast< common::CIntroduceEvent const* >( simple_state::triggering_event() );

		db.Add(requestedEvent->m_address);
		context< CAcceptNodeAction >().setRequest( 0 );
	}
};


struct CBothUnidentifiedConnecting : boost::statechart::state< CBothUnidentifiedConnecting, CAcceptNodeAction >
{
	CBothUnidentifiedConnecting( my_context ctx ) : my_base( ctx )
	{

		common::CNodeConnectedEvent const* connectedEvent = dynamic_cast< common::CNodeConnectedEvent const* >( simple_state::triggering_event() );
		context< CAcceptNodeAction >().setMediumKind( convertToInt( connectedEvent->m_node ) );
		// looks funny that  I set it in this  state, but let  it  be
		CSeedNodesManager::getInstance()->addNode( connectedEvent->m_node );
		context< CAcceptNodeAction >().setRequest( new common::CIdentifyRequest<SeedResponses>( convertToInt( connectedEvent->m_node ), context< CAcceptNodeAction >().getPayload() ) );

	}

	boost::statechart::result react( const common::CContinueEvent & _continueEvent )
	{
		context< CAcceptNodeAction >().setRequest( new common::CContinueReqest<SeedResponses>( _continueEvent.m_keyId, context< CAcceptNodeAction >().getMediumKind() ) );
	}

	typedef boost::mpl::list<
	boost::statechart::transition< common::CIntroduceEvent, CPairIdentifiedConnecting >,
	boost::statechart::custom_reaction< common::CContinueEvent >
	> reactions;
};

struct CBothUnidentifiedConnected : boost::statechart::state< CBothUnidentifiedConnected, CAcceptNodeAction >
{
	CBothUnidentifiedConnected( my_context ctx ) : my_base( ctx )
	{
		common::CIntroduceEvent const* requestedEvent = dynamic_cast< common::CIntroduceEvent const* >( simple_state::triggering_event() );

		createIdentifyResponse( context< CAcceptNodeAction >() );

	}

	boost::statechart::result react( const common::CContinueEvent & _continueEvent )
	{
		context< CAcceptNodeAction >().setRequest( new common::CContinueReqest<SeedResponses>( _continueEvent.m_keyId, context< CAcceptNodeAction >().getMediumKind() ) );
	}

	typedef boost::mpl::list<
	boost::statechart::transition< common::CIntroduceEvent, CPairIdentifiedConnected >,
	boost::statechart::custom_reaction< common::CContinueEvent >
	> reactions;
};

struct CCantReachNode : boost::statechart::state< CCantReachNode, CAcceptNodeAction >
{
	CCantReachNode( my_context ctx ) : my_base( ctx )
	{
		context< CAcceptNodeAction >().setValid( false );
		context< CAcceptNodeAction >().setRequest( 0 );
	}
};

struct CUnconnected : boost::statechart::state< CUnconnected, CAcceptNodeAction >
{
	CUnconnected( my_context ctx ) : my_base( ctx )
	{
		context< CAcceptNodeAction >().setRequest(
				  new common::CConnectToNodeRequest< SeedResponses >( std::string(""), context< CAcceptNodeAction >().getAddress(), 0 ) );

	}

	typedef boost::mpl::list<
	boost::statechart::transition< common::CNodeConnectedEvent, CBothUnidentifiedConnecting >,
	boost::statechart::transition< common::CCantReachNode, CBothUnidentifiedConnecting >
	> reactions;

};

struct CSynchronizing : boost::statechart::simple_state< CSynchronizing, CAcceptNodeAction >
{

};

CAcceptNodeAction::CAcceptNodeAction( std::vector< unsigned char > const & _payload, unsigned int _mediumKind )
: common::CAction< SeedResponses >( false )
, m_payload( _payload )
, m_request( 0 )
, m_passive( true )
, m_mediumKind( _mediumKind )
, m_valid( false )
{
	initiate();
	process_event( common::CSwitchToConnectedEvent() );
}

CAcceptNodeAction::CAcceptNodeAction( CAddress const & _nodeAddress )
	: m_nodeAddress( _nodeAddress )
	, m_request( 0 )
	, m_passive( false )
	, m_valid( false )
{
	for ( unsigned int i = 0; i < ms_randomPayloadLenght; i++ )
	{
		m_payload.push_back( insecure_rand() % 256 );
	}
	initiate();
	process_event( common::CSwitchToConnectingEvent() );
}

common::CRequest< SeedResponses >*
CAcceptNodeAction::execute()
{
	return m_request;
}

void
CAcceptNodeAction::accept( common::CSetResponseVisitor< SeedResponses > & _visitor )
{
	_visitor.visit( *this );
}

void
CAcceptNodeAction::setRequest( common::CRequest< SeedResponses >* _request )
{
	m_request = _request;
}

CAddress
CAcceptNodeAction::getAddress() const
{
	return m_nodeAddress;
}

std::vector< unsigned char >
CAcceptNodeAction::getPayload() const
{
	return m_payload;
}

unsigned int
CAcceptNodeAction::getMediumKind() const
{
	return m_mediumKind;
}

void
CAcceptNodeAction::setMediumKind( unsigned int _mediumKind )
{
	m_mediumKind = _mediumKind;
}



}
