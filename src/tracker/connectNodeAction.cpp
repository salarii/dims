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


struct CConnectNodeActionUninitiated : boost::statechart::simple_state< CConnectNodeActionUninitiated, CConnectNodeAction >
{
	typedef boost::mpl::list<
	boost::statechart::transition< common::CSwitchToConnectingEvent, CUnconnected >,
	boost::statechart::transition< common::CSwitchToConnectedEvent, CBothUnidentifiedConnected >
	> reactions;

};

class ConnectedToSeed;

class ConnectedToMonitor;

class ConnectedToTracker;
// merge  this  with CPairIdentifiedConnected ???
struct CIdentified : boost::statechart::state< CIdentified, CConnectNodeAction >
{
	CIdentified( my_context ctx ) : my_base( ctx )
	{
	}

	boost::statechart::result react( common::CContinueEvent const & _continueEvent )
	{
		context< CConnectNodeAction >().setRequest( new common::CContinueReqest<TrackerResponses>( _continueEvent.m_keyId, context< CConnectNodeAction >().getMediumKind() ) );
	}

	boost::statechart::result react( common::CRoleEvent const & _roleEvent )
	{
		switch ( _roleEvent.m_role )
		{
		case common::CRole::Tracker:
			transit< ConnectedToTracker >();
			break;
		case common::CRole::Seed:
			transit< ConnectedToSeed >();
			break;
		case common::CRole::Monitor:
			transit< ConnectedToMonitor >();
			break;
		default:
			break;
		}
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CRoleEvent >,
	boost::statechart::custom_reaction< common::CContinueEvent >
	> reactions;
};


template < class Parent >
void
createIdentifyResponse( Parent & parent )
{
	uint256 hash = Hash( &parent.getPayload().front(), &parent.getPayload().back() );

	std::vector< unsigned char > signedHash;
	common::CAuthenticationProvider::getInstance()->sign( hash, signedHash );

	parent.setRequest( new common::CIdentifyResponse<TrackerResponses>( parent.getMediumKind(), signedHash, common::CAuthenticationProvider::getInstance()->getMyKey(), parent.getPayload(), parent.getActionKey() ) );
}

struct CPairIdentifiedConnecting : boost::statechart::state< CPairIdentifiedConnecting, CConnectNodeAction >
{
	CPairIdentifiedConnecting( my_context ctx ) : my_base( ctx )
	{
		common::CIntroduceEvent const* requestedEvent = dynamic_cast< common::CIntroduceEvent const* >( simple_state::triggering_event() );

		uint256 hash = Hash( &requestedEvent->m_payload.front(), &requestedEvent->m_payload.back() );

		if ( requestedEvent->m_key.Verify( hash, requestedEvent->m_signed ) )
		{
			context< CConnectNodeAction >().setPublicKey( requestedEvent->m_key );

			CTrackerNodesManager::getInstance()->setPublicKey( context< CConnectNodeAction >().getServiceAddress(), requestedEvent->m_key );
			createIdentifyResponse( context< CConnectNodeAction >() );
		}
		else
		{
			context< CConnectNodeAction >().setRequest( 0 );
		}
		// something  is  wrong  with  pair react  somehow for  now put 0

	}

	typedef boost::mpl::list<
	boost::statechart::transition< common::CContinueEvent, CIdentified >// kind of using side effect is this ok??
	> reactions;

};

struct CPairIdentifiedConnected : boost::statechart::state< CPairIdentifiedConnected, CConnectNodeAction >
{
	CPairIdentifiedConnected( my_context ctx ) : my_base( ctx )
	{
		common::CIntroduceEvent const* requestedEvent = dynamic_cast< common::CIntroduceEvent const* >( simple_state::triggering_event() );

	}

	boost::statechart::result react( common::CContinueEvent const & _continueEvent )
	{
		context< CConnectNodeAction >().setRequest( new common::CContinueReqest<TrackerResponses>( _continueEvent.m_keyId, context< CConnectNodeAction >().getMediumKind() ) );
	}

	boost::statechart::result react( common::CRoleEvent const & _roleEvent )
	{
		context< CConnectNodeAction >().setRequest( new common::CNetworkRoleRequest<TrackerResponses>( context< CConnectNodeAction >().getActionKey(), common::CRole::Tracker, context< CConnectNodeAction >().getMediumKind() ) );

		switch ( _roleEvent.m_role )
		{
		case common::CRole::Tracker:
			transit< ConnectedToTracker >();
			break;
		case common::CRole::Seed:
			transit< ConnectedToSeed >();
			break;
		case common::CRole::Monitor:
			transit< ConnectedToMonitor >();
			break;
		default:
			break;
		}
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CRoleEvent >,
	boost::statechart::custom_reaction< common::CContinueEvent >
	> reactions;
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

struct ConnectedToTracker : boost::statechart::state< ConnectedToTracker, CConnectNodeAction >
{
	ConnectedToTracker( my_context ctx ) : my_base( ctx )
	{
		context< CConnectNodeAction >().setRequest( 0 );

		CTrackerNodesManager::getInstance()->setValidNode( common::CValidNodeInfo( context< CConnectNodeAction >().getPublicKey().GetID(), context< CConnectNodeAction >().getServiceAddress(), common::CRole::Tracker ) );

		common::CAuthenticationProvider::getInstance()->addPubKey( context< CConnectNodeAction >().getPublicKey() );
	}

};

struct CStop;

struct ConnectedToSeed : boost::statechart::state< ConnectedToSeed, CConnectNodeAction >
{
	ConnectedToSeed( my_context ctx ) : my_base( ctx )
	{

	}

	boost::statechart::result react( const common::CContinueEvent & _continueEvent )
	{
		context< CConnectNodeAction >().setRequest( new common::CContinueReqest<TrackerResponses>( _continueEvent.m_keyId, context< CConnectNodeAction >().getMediumKind() ) );
	}

	boost::statechart::result react( const common::CAckEvent & _ackEvent )
	{
		std::vector< common::CValidNodeInfo > validNodesInfo;

		BOOST_FOREACH( common::CValidNodeInfo const & validNodeInfo, CTrackerNodesManager::getInstance()->getValidNodes() )
		{
			validNodesInfo.push_back( validNodeInfo );
		}
		context< CConnectNodeAction >().setRequest( new common::CKnownNetworkInfoRequest< TrackerResponses >( context< CConnectNodeAction >().getActionKey(), validNodesInfo, context< CConnectNodeAction >().getMediumKind() ) );

		transit< CStop >();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CContinueEvent >,
	boost::statechart::custom_reaction< common::CAckEvent >
	> reactions;
};

struct CStop : boost::statechart::state< CStop, CConnectNodeAction >
{
	CStop( my_context ctx ) : my_base( ctx )
	{
	}

	boost::statechart::result react( const common::CContinueEvent & _continueEvent )
	{
		context< CConnectNodeAction >().setRequest( 0 );
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CContinueEvent >
	> reactions;
};


struct ConnectedToMonitor : boost::statechart::state< ConnectedToMonitor, CConnectNodeAction >
{
	ConnectedToMonitor( my_context ctx ) : my_base( ctx )
	{
	}
};

CConnectNodeAction::CConnectNodeAction( uint256 const & _actionKey, std::vector< unsigned char > const & _payload, unsigned int _mediumKind )
: CCommunicationAction( _actionKey )
, m_payload( _payload )
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

std::vector< unsigned char >  const &
CConnectNodeAction::getPayload() const
{
	return m_payload;
}

unsigned int
CConnectNodeAction::getMediumKind() const
{
	return m_mediumKind;
}

CPubKey
CConnectNodeAction::getPublicKey() const
{
	return m_key;
}

void
CConnectNodeAction::setPublicKey( CPubKey const & _pubKey )
{
	m_key = _pubKey;
}


void
CConnectNodeAction::setMediumKind( unsigned int _mediumKind )
{
	m_mediumKind = _mediumKind;
}

}
