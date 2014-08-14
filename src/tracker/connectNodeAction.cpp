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
#include "trackerFilters.h"

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

template < class Parent >
void
createIdentifyResponse( Parent & parent )
{
	uint256 hash = Hash( &parent.getPayload().front(), &parent.getPayload().back() );

	std::vector< unsigned char > signedHash;
	common::CAuthenticationProvider::getInstance()->sign( hash, signedHash );

	parent.setRequest( new common::CIdentifyResponse<TrackerResponses>( new CSpecificMediumFilter( parent.getMediumPtr() ), signedHash, common::CAuthenticationProvider::getInstance()->getMyKey(), parent.getPayload(), parent.getActionKey() ) );
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
		// something  is  wrong  with  pair react  somehow for  now put 0
			context< CConnectNodeAction >().setRequest( 0 );
		}
	}

	boost::statechart::result react( common::CContinueEvent const & _continueEvent )
	{
		context< CConnectNodeAction >().setRequest( new common::CContinueReqest<TrackerResponses>( _continueEvent.m_keyId, new CSpecificMediumFilter( context< CConnectNodeAction >().getMediumPtr() ) ) );
	}

	boost::statechart::result react( common::CRoleEvent const & _roleEvent )
	{
		context< CConnectNodeAction >().setRequest( new common::CNetworkRoleRequest<TrackerResponses>( context< CConnectNodeAction >().getActionKey(), common::CRole::Tracker, new CSpecificMediumFilter( context< CConnectNodeAction >().getMediumPtr() ) ) );

		switch ( _roleEvent.m_role )
		{
		case common::CRole::Tracker:
			return transit< ConnectedToTracker >();
		case common::CRole::Seed:
			return transit< ConnectedToSeed >();
		case common::CRole::Monitor:
			return transit< ConnectedToMonitor >();
		default:
			break;
		}
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CRoleEvent >,
	boost::statechart::custom_reaction< common::CContinueEvent >
	> reactions;

};

struct CPairIdentifiedConnected : boost::statechart::state< CPairIdentifiedConnected, CConnectNodeAction >
{
	CPairIdentifiedConnected( my_context ctx ) : my_base( ctx )
	{
		common::CIntroduceEvent const* requestedEvent = dynamic_cast< common::CIntroduceEvent const* >( simple_state::triggering_event() );

		uint256 hash = Hash( &requestedEvent->m_payload.front(), &requestedEvent->m_payload.back() );

		if ( requestedEvent->m_key.Verify( hash, requestedEvent->m_signed ) )
		{
			CTrackerNodesManager::getInstance()->setPublicKey( requestedEvent->m_address, requestedEvent->m_key );

			context< CConnectNodeAction >().setRequest( new common::CNetworkRoleRequest<TrackerResponses>( context< CConnectNodeAction >().getActionKey(), common::CRole::Tracker, new CSpecificMediumFilter( context< CConnectNodeAction >().getMediumPtr() ) ) );
		}
		else
		{
			context< CConnectNodeAction >().setRequest( 0 );
		}
	}



	boost::statechart::result react( common::CContinueEvent const & _continueEvent )
	{
		context< CConnectNodeAction >().setRequest( new common::CContinueReqest<TrackerResponses>( _continueEvent.m_keyId, new CSpecificMediumFilter( context< CConnectNodeAction >().getMediumPtr() ) ) );
	}

	boost::statechart::result react( common::CRoleEvent const & _roleEvent )
	{
		switch ( _roleEvent.m_role )
		{
		case common::CRole::Tracker:
			return transit< ConnectedToTracker >();
		case common::CRole::Seed:
			context< CConnectNodeAction >().setRequest( new common::CAckRequest<TrackerResponses>( context< CConnectNodeAction >().getActionKey(), new CSpecificMediumFilter( context< CConnectNodeAction >().getMediumPtr() ) ) );
			return transit< ConnectedToSeed >();
		case common::CRole::Monitor:
			return transit< ConnectedToMonitor >();
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
		context< CConnectNodeAction >().setMediumPtr( convertToInt( connectedEvent->m_node ) );
		// looks funny that  I set it in this  state, but let  it  be
		CTrackerNodesManager::getInstance()->addNode( connectedEvent->m_node );

		context< CConnectNodeAction >().setRequest( new common::CIdentifyRequest<TrackerResponses>( new CSpecificMediumFilter( convertToInt( connectedEvent->m_node ) ), context< CConnectNodeAction >().getPayload(), context< CConnectNodeAction >().getActionKey() ) );

	}

	boost::statechart::result react( const common::CContinueEvent & _continueEvent )
	{
		context< CConnectNodeAction >().setRequest( new common::CContinueReqest<TrackerResponses>( _continueEvent.m_keyId, new CSpecificMediumFilter( context< CConnectNodeAction >().getMediumPtr() ) ) );
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
		context< CConnectNodeAction >().setRequest( new common::CContinueReqest<TrackerResponses>( _continueEvent.m_keyId, new CSpecificMediumFilter( context< CConnectNodeAction >().getMediumPtr() ) ) );
	}

	typedef boost::mpl::list<
	boost::statechart::transition< common::CIntroduceEvent, CPairIdentifiedConnected >,
	boost::statechart::custom_reaction< common::CContinueEvent >
	> reactions;
};

// it is  wrong but  for  now...
struct CCantReachNode : boost::statechart::state< CCantReachNode, CConnectNodeAction >
{
	CCantReachNode( my_context ctx ) : my_base( ctx )
	{
		context< CConnectNodeAction >().setRequest( 0 );
	}
};

struct CUnconnected : boost::statechart::state< CUnconnected, CConnectNodeAction >
{
	CUnconnected( my_context ctx ) : my_base( ctx )
	{
		context< CConnectNodeAction >().setRequest(
				  new CConnectToTrackerRequest( context< CConnectNodeAction >().getAddress(), context< CConnectNodeAction >().getServiceAddress() ) );
	}

	typedef boost::mpl::list<
	boost::statechart::transition< common::CNodeConnectedEvent, CBothUnidentifiedConnecting >,
	boost::statechart::transition< common::CCantReachNode, CCantReachNode >
	> reactions;

};

struct ConnectedToTracker : boost::statechart::state< ConnectedToTracker, CConnectNodeAction >
{
	ConnectedToTracker( my_context ctx ) : my_base( ctx )
	{
		context< CConnectNodeAction >().setRequest( 0 );

		CTrackerNodesManager::getInstance()->setValidNode( common::CValidNodeInfo( context< CConnectNodeAction >().getPublicKey(), context< CConnectNodeAction >().getServiceAddress(), common::CRole::Tracker ) );

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
		context< CConnectNodeAction >().setRequest( new common::CContinueReqest<TrackerResponses>( _continueEvent.m_keyId, new CSpecificMediumFilter( context< CConnectNodeAction >().getMediumPtr() ) ) );
	}

	boost::statechart::result react( const common::CAckEvent & _ackEvent )
	{
		std::vector< common::CValidNodeInfo > validNodesInfo;

		BOOST_FOREACH( common::CValidNodeInfo const & validNodeInfo, CTrackerNodesManager::getInstance()->getValidNodes() )
		{
			validNodesInfo.push_back( validNodeInfo );
		}
		context< CConnectNodeAction >().setRequest( new common::CKnownNetworkInfoRequest< TrackerResponses >( context< CConnectNodeAction >().getActionKey(), validNodesInfo, new CSpecificMediumFilter( context< CConnectNodeAction >().getMediumPtr() ) ) );

		return transit< CStop >();
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

CConnectNodeAction::CConnectNodeAction( uint256 const & _actionKey, std::vector< unsigned char > const & _payload, long long unsigned _mediumPtr )
: CCommunicationAction( _actionKey )
, m_payload( _payload )
, m_request( 0 )
, m_passive( true )
, m_mediumPtr( _mediumPtr )
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

long long unsigned
CConnectNodeAction::getMediumPtr() const
{
	return m_mediumPtr;
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
CConnectNodeAction::setMediumPtr( long long unsigned _mediumPtr )
{
	m_mediumPtr = _mediumPtr;
}

}
