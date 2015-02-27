// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "acceptNodeAction.h"
#include "common/setResponseVisitor.h"
#include "common/commonEvents.h"
#include "common/authenticationProvider.h"
#include "common/mediumRequests.h"

#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/state.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/custom_reaction.hpp>

#include "acceptNodeAction.h"
#include "seedNodesManager.h"
#include "seedNodeMedium.h"

#include "seedDb.h"
#include "seedFilter.h"

// ugly as hell, refactor as soon as possible
namespace seed
{
extern CAddrDb db;

struct CUnconnected; struct CBothUnidentifiedConnected; struct CCantReachNode;

uint64_t const WaitTime = 10;

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
		context< CAcceptNodeAction >().dropRequests();
	}
};


template < class Parent >
void
createIdentifyResponse( Parent & parent )
{
	uint256 hash = Hash( &parent.getPayload().front(), &parent.getPayload().back() );

	std::vector< unsigned char > signedHash;
	common::CAuthenticationProvider::getInstance()->sign( hash, signedHash );

//	parent.addRequests( new common::CIdentifyResponse<SeedResponses>( new CSpecificMediumFilter( parent.getMediumPtr() ), signedHash, common::CAuthenticationProvider::getInstance()->getMyKey(), parent.getPayload(), parent.getActionKey() ) );
}

struct ConnectedToTracker;
struct ConnectedToSeed;
struct ConnectedToMonitor;

struct CDetermineRoleConnecting : boost::statechart::state< CDetermineRoleConnecting, CAcceptNodeAction >
{
	CDetermineRoleConnecting( my_context ctx ) : my_base( ctx )
	{
		m_time = GetTime();
		context< CAcceptNodeAction >().dropRequests();
		context< CAcceptNodeAction >().addRequests( new common::CNetworkRoleRequest<SeedResponses>( context< CAcceptNodeAction >().getActionKey(), common::CRole::Seed, new CSpecificMediumFilter( context< CAcceptNodeAction >().getMediumPtr() ) ) );
	}

	boost::statechart::result react( common::CRoleEvent const & _roleEvent )
	{
		context< CAcceptNodeAction >().dropRequests();
		context< CAcceptNodeAction >().addRequests( new common::CAckRequest< SeedResponses >( context< CAcceptNodeAction >().getActionKey(), new CSpecificMediumFilter( context< CAcceptNodeAction >().getMediumPtr() ) ) );

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
		return discard_event();
	}

	boost::statechart::result react( common::CAckEvent const & _ackEvent )
	{
		context< CAcceptNodeAction >().dropRequests();
		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CRoleEvent >,
	boost::statechart::custom_reaction< common::CAckEvent >
	> reactions;

	uint64_t m_time;
};


struct CPairIdentifiedConnecting : boost::statechart::state< CPairIdentifiedConnecting, CAcceptNodeAction >
{
	CPairIdentifiedConnecting( my_context ctx ) : my_base( ctx )
	{
		m_time = GetTime();

		common::CIdentificationResult const* requestedEvent = dynamic_cast< common::CIdentificationResult const* >( simple_state::triggering_event() );

		uint256 hash = Hash( &requestedEvent->m_payload.front(), &requestedEvent->m_payload.back() );

		if ( requestedEvent->m_key.Verify( hash, requestedEvent->m_signed ) )
		{
			context< CAcceptNodeAction >().dropRequests();
			context< CAcceptNodeAction >().addRequests( new common::CAckRequest< SeedResponses >( context< CAcceptNodeAction >().getActionKey(), new CSpecificMediumFilter( context< CAcceptNodeAction >().getMediumPtr() ) ) );
			createIdentifyResponse( context< CAcceptNodeAction >() );
		}
		else
		{
			context< CAcceptNodeAction >().dropRequests();
		}
	}

	typedef boost::mpl::list<
	boost::statechart::transition< common::CAckEvent, CDetermineRoleConnecting >
	> reactions;

	int m_role;
	uint64_t m_time;
};

struct CDetermineRoleConnected : boost::statechart::state< CDetermineRoleConnected, CAcceptNodeAction >
{
	CDetermineRoleConnected( my_context ctx ) : my_base( ctx )
	{
		m_time = GetTime();
		context< CAcceptNodeAction >().dropRequests();
	}

	boost::statechart::result react( common::CRoleEvent const & _roleEvent )
	{
		m_role = _roleEvent.m_role;
		context< CAcceptNodeAction >().dropRequests();
		context< CAcceptNodeAction >().addRequests( new common::CAckRequest< SeedResponses >( context< CAcceptNodeAction >().getActionKey(), new CSpecificMediumFilter( context< CAcceptNodeAction >().getMediumPtr() ) ) );
		context< CAcceptNodeAction >().addRequests( new common::CNetworkRoleRequest<SeedResponses>( context< CAcceptNodeAction >().getActionKey(), common::CRole::Seed, new CSpecificMediumFilter( context< CAcceptNodeAction >().getMediumPtr() ) ) );
		return discard_event();
	}

	boost::statechart::result react( common::CAckEvent const & _ackEvent )
	{
		switch ( m_role )
		{
		case common::CRole::Tracker:
			db.Add( context< CAcceptNodeAction >().getAddress() );
			return transit< ConnectedToTracker >();
		case common::CRole::Seed:
			return transit< ConnectedToSeed >();
		case common::CRole::Monitor:
			db.Add( context< CAcceptNodeAction >().getAddress() );
			return transit< ConnectedToMonitor >();
		default:
			break;
		}
		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CRoleEvent >,
	boost::statechart::custom_reaction< common::CAckEvent >
	> reactions;

	int m_role;
	uint64_t m_time;
};

struct CPairIdentifiedConnected : boost::statechart::state< CPairIdentifiedConnected, CAcceptNodeAction >
{
	CPairIdentifiedConnected( my_context ctx ) : my_base( ctx )
	{
		m_time = GetTime();
		context< CAcceptNodeAction >().dropRequests();
	}

	boost::statechart::result react( const common::CIdentificationResult & _identificationResult )
	{
		uint256 hash = Hash( &_identificationResult.m_payload.front(), &_identificationResult.m_payload.back() );

		if ( _identificationResult.m_key.Verify( hash, _identificationResult.m_signed ) )
		{
			CSeedNodesManager::getInstance()->setPublicKey( _identificationResult.m_address, _identificationResult.m_key );
			context< CAcceptNodeAction >().dropRequests();
			context< CAcceptNodeAction >().addRequests( new common::CAckRequest< SeedResponses >( context< CAcceptNodeAction >().getActionKey(), new CSpecificMediumFilter( context< CAcceptNodeAction >().getMediumPtr() ) ) );
			context< CAcceptNodeAction >().setAddress( _identificationResult.m_address );
		}
		else
		{
			context< CAcceptNodeAction >().setValid( false );
			context< CAcceptNodeAction >().dropRequests();
		}
		return transit< CDetermineRoleConnected >();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CIdentificationResult >
	> reactions;

	uint64_t m_time;
};

struct CBothUnidentifiedConnecting : boost::statechart::state< CBothUnidentifiedConnecting, CAcceptNodeAction >
{
	CBothUnidentifiedConnecting( my_context ctx ) : my_base( ctx )
	{
		m_time = GetTime();

		common::CNodeConnectedEvent const* connectedEvent = dynamic_cast< common::CNodeConnectedEvent const* >( simple_state::triggering_event() );
		context< CAcceptNodeAction >().setMediumPtr( convertToInt( connectedEvent->m_node ) );
		CSeedNodesManager::getInstance()->addNode( new CSeedNodeMedium( connectedEvent->m_node ) );
		context< CAcceptNodeAction >().dropRequests();
//		context< CAcceptNodeAction >().addRequests( new common::CIdentifyRequest<SeedResponses>( new CSpecificMediumFilter( convertToInt( connectedEvent->m_node ) ), context< CAcceptNodeAction >().getPayload(), context< CAcceptNodeAction >().getActionKey() ) );

	}

	boost::statechart::result react( common::CAckEvent const & _ackEvent )
	{
		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::transition< common::CIdentificationResult, CPairIdentifiedConnecting >,
	boost::statechart::custom_reaction< common::CAckEvent >
	> reactions;

	uint64_t m_time;
};

struct CBothUnidentifiedConnected : boost::statechart::state< CBothUnidentifiedConnected, CAcceptNodeAction >
{
	CBothUnidentifiedConnected( my_context ctx ) : my_base( ctx )
	{
		m_time = GetTime();

		context< CAcceptNodeAction >().dropRequests();
		context< CAcceptNodeAction >().addRequests( new common::CAckRequest<SeedResponses>( context< CAcceptNodeAction >().getActionKey(), new CSpecificMediumFilter( context< CAcceptNodeAction >().getMediumPtr() ) ) );
		createIdentifyResponse( context< CAcceptNodeAction >() );
	}

	typedef boost::mpl::list<
	boost::statechart::transition< common::CAckEvent, CPairIdentifiedConnected >
	> reactions;

	uint64_t m_time;
};

struct CCantReachNode : boost::statechart::state< CCantReachNode, CAcceptNodeAction >
{
	CCantReachNode( my_context ctx ) : my_base( ctx )
	{
		context< CAcceptNodeAction >().setValid( false );
		context< CAcceptNodeAction >().dropRequests();
	}
};

struct CUnconnected : boost::statechart::state< CUnconnected, CAcceptNodeAction >
{
	CUnconnected( my_context ctx ) : my_base( ctx )
	{
		context< CAcceptNodeAction >().dropRequests();
		context< CAcceptNodeAction >().addRequests(
					new common::CConnectToNodeRequest< SeedResponses >( std::string(""), context< CAcceptNodeAction >().getAddress(), new CMediumClassFilter( common::CMediumKinds::Internal ) ) );
	}

	typedef boost::mpl::list<
	boost::statechart::transition< common::CNodeConnectedEvent, CBothUnidentifiedConnecting >,
	boost::statechart::transition< common::CCantReachNode, CCantReachNode >
	> reactions;

};

struct ConnectedToTracker : boost::statechart::state< ConnectedToTracker, CAcceptNodeAction >
{
	ConnectedToTracker( my_context ctx ) : my_base( ctx )
	{
		m_time = GetTime();

		context< CAcceptNodeAction >().setValid( true );

		context< CAcceptNodeAction >().dropRequests();
		context< CAcceptNodeAction >().addRequests(
					new common::CKnownNetworkInfoRequest< SeedResponses >( context< CAcceptNodeAction >().getActionKey(), common::CKnownNetworkInfo(), new CSpecificMediumFilter( context< CAcceptNodeAction >().getMediumPtr() ) ) );// vicious usage of CKnownNetworkInfoRequest
		context< CAcceptNodeAction >().addRequests( new common::CTimeEventRequest<SeedResponses>( 20000, new CMediumClassFilter( common::CMediumKinds::Time ) ) );
	}

	boost::statechart::result react( common::CNetworkInfoEvent const & _networkInfo )
	{
		context< CAcceptNodeAction >().dropRequests();

		BOOST_FOREACH( common::CValidNodeInfo validNodeInfo, _networkInfo.m_trackersInfo )
		{
				db.Add( validNodeInfo.m_address );
		}

		BOOST_FOREACH( common::CValidNodeInfo validNodeInfo, _networkInfo.m_monitorsInfo )
		{
				db.Add( validNodeInfo.m_address );
		}
		return discard_event();
	}

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		context< CAcceptNodeAction >().dropRequests();
		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CTimeEvent >,
	boost::statechart::custom_reaction< common::CNetworkInfoEvent >
	> reactions;

	uint64_t m_time;
};

struct ConnectedToSeed : boost::statechart::state< ConnectedToSeed, CAcceptNodeAction >
{
	ConnectedToSeed( my_context ctx ) : my_base( ctx )
	{
		context< CAcceptNodeAction >().dropRequests();
	}
};


struct ConnectedToMonitor : boost::statechart::state< ConnectedToMonitor, CAcceptNodeAction >
{
	ConnectedToMonitor( my_context ctx ) : my_base( ctx )
	{

		context< CAcceptNodeAction >().setValid( true );

		context< CAcceptNodeAction >().dropRequests();
		context< CAcceptNodeAction >().addRequests(
					new common::CKnownNetworkInfoRequest< SeedResponses >( context< CAcceptNodeAction >().getActionKey(), common::CKnownNetworkInfo(), new CSpecificMediumFilter( context< CAcceptNodeAction >().getMediumPtr() ) ) );// vicious usage of CKnownNetworkInfoRequest
		context< CAcceptNodeAction >().addRequests( new common::CTimeEventRequest<SeedResponses>( 20000, new CMediumClassFilter( common::CMediumKinds::Time ) ) );
	}

	boost::statechart::result react( common::CNetworkInfoEvent const & _networkInfo )
	{
		context< CAcceptNodeAction >().dropRequests();

		BOOST_FOREACH( common::CValidNodeInfo validNodeInfo, _networkInfo.m_trackersInfo )
		{
				db.Add( validNodeInfo.m_address );
		}

		BOOST_FOREACH( common::CValidNodeInfo validNodeInfo, _networkInfo.m_monitorsInfo )
		{
				db.Add( validNodeInfo.m_address );
		}
		return discard_event();
	}

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		context< CAcceptNodeAction >().dropRequests();
		return discard_event();
	}

	typedef boost::mpl::list<
			boost::statechart::custom_reaction< common::CTimeEvent >,
			boost::statechart::custom_reaction< common::CNetworkInfoEvent >
	> reactions;

	uint64_t m_time;
};


struct CSynchronizing : boost::statechart::simple_state< CSynchronizing, CAcceptNodeAction >
{

};

CAcceptNodeAction::CAcceptNodeAction( uint256 const & _actionKey, uintptr_t _mediumPtr )
	: common::CCommunicationAction( _actionKey )
	, m_passive( true )
	, m_mediumPtr( _mediumPtr )
	, m_valid( false )
{
	initiate();
	process_event( common::CSwitchToConnectedEvent() );
}

CAcceptNodeAction::CAcceptNodeAction( CAddress const & _nodeAddress )
	: common::CAction< SeedResponses >( false )
	, m_nodeAddress( _nodeAddress )
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

void
CAcceptNodeAction::accept( common::CSetResponseVisitor< SeedResponses > & _visitor )
{
	_visitor.visit( *this );
}

CAddress
CAcceptNodeAction::getAddress() const
{
	return m_nodeAddress;
}

void
CAcceptNodeAction::setAddress( CAddress const & _address )
{
	m_nodeAddress = _address;
}

std::vector< unsigned char > const &
CAcceptNodeAction::getPayload() const
{
	return m_payload;
}

uintptr_t
CAcceptNodeAction::getMediumPtr() const
{
	return m_mediumPtr;
}

void
CAcceptNodeAction::setMediumPtr( uintptr_t _mediumPtr )
{
	m_mediumPtr = _mediumPtr;
}

}
