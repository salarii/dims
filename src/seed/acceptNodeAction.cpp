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

	parent.setRequest( new common::CIdentifyResponse<SeedResponses>( new CSpecificMediumFilter( parent.getMediumPtr() ), signedHash, common::CAuthenticationProvider::getInstance()->getMyKey(), parent.getPayload(), parent.getActionKey() ) );
}

struct ConnectedToTracker;
struct ConnectedToSeed;
struct ConnectedToMonitor;

struct CDetermineRoleConnecting : boost::statechart::state< CDetermineRoleConnecting, CAcceptNodeAction >
{
	CDetermineRoleConnecting( my_context ctx ) : my_base( ctx )
	{
		m_time = GetTime();
		context< CAcceptNodeAction >().setRequest( new common::CNetworkRoleRequest<SeedResponses>( context< CAcceptNodeAction >().getActionKey(), common::CRole::Seed, new CSpecificMediumFilter( context< CAcceptNodeAction >().getMediumPtr() ) ) );
	}

	boost::statechart::result react( common::CRoleEvent const & _roleEvent )
	{
		m_role = _roleEvent.m_role;
		context< CAcceptNodeAction >().setRequest( new common::CAckRequest< SeedResponses >( context< CAcceptNodeAction >().getActionKey(), new CSpecificMediumFilter( context< CAcceptNodeAction >().getMediumPtr() ) ) );
		return discard_event();
	}

	boost::statechart::result react( common::CContinueEvent const & _continueEvent )
	{
		if ( GetTime() - m_time > WaitTime )
		{
			return transit< CCantReachNode >();
		}
		else
		{
			context< CAcceptNodeAction >().setRequest( new common::CContinueReqest<SeedResponses>( _continueEvent.m_keyId, new CSpecificMediumFilter( context< CAcceptNodeAction >().getMediumPtr() ) ) );
			return discard_event();
		}
	}

	boost::statechart::result react( common::CAckPromptResult const & _promptAck )
	{
		switch ( m_role )
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
		context< CAcceptNodeAction >().setRequest( new common::CContinueReqest<SeedResponses>( context< CAcceptNodeAction >().getActionKey(), new CSpecificMediumFilter( context< CAcceptNodeAction >().getMediumPtr() ) ) );
		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CRoleEvent >,
	boost::statechart::custom_reaction< common::CContinueEvent >,
	boost::statechart::custom_reaction< common::CAckPromptResult >,
	boost::statechart::custom_reaction< common::CAckEvent >
	> reactions;

	int m_role;
	uint64_t m_time;
};


struct CPairIdentifiedConnecting : boost::statechart::state< CPairIdentifiedConnecting, CAcceptNodeAction >
{
	CPairIdentifiedConnecting( my_context ctx ) : my_base( ctx )
	{
		m_time = GetTime();

		common::CIntroduceEvent const* requestedEvent = dynamic_cast< common::CIntroduceEvent const* >( simple_state::triggering_event() );

		uint256 hash = Hash( &requestedEvent->m_payload.front(), &requestedEvent->m_payload.back() );

		if ( requestedEvent->m_key.Verify( hash, requestedEvent->m_signed ) )
		{
			context< CAcceptNodeAction >().setRequest( new common::CAckRequest< SeedResponses >( context< CAcceptNodeAction >().getActionKey(), new CSpecificMediumFilter( context< CAcceptNodeAction >().getMediumPtr() ) ) );
		}
		else
		{
			context< CAcceptNodeAction >().setRequest( 0 );
		}
	}

	boost::statechart::result react( common::CContinueEvent const & _continueEvent )
	{
		if ( GetTime() - m_time > WaitTime )
		{
			return transit< CCantReachNode >();
		}
		else
		{
			context< CAcceptNodeAction >().setRequest( new common::CContinueReqest<SeedResponses>( _continueEvent.m_keyId, new CSpecificMediumFilter( context< CAcceptNodeAction >().getMediumPtr() ) ) );
			return discard_event();
		}
	}

	boost::statechart::result react( common::CAckPromptResult const & _roleEvent )
	{
		createIdentifyResponse( context< CAcceptNodeAction >() );
		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CContinueEvent >,
	boost::statechart::custom_reaction< common::CAckPromptResult >,
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
		context< CAcceptNodeAction >().setRequest( new common::CContinueReqest<SeedResponses>( context< CAcceptNodeAction >().getActionKey(), new CSpecificMediumFilter( context< CAcceptNodeAction >().getMediumPtr() ) ) );
	}

	boost::statechart::result react( common::CRoleEvent const & _roleEvent )
	{
		m_role = _roleEvent.m_role;
		context< CAcceptNodeAction >().setRequest( new common::CAckRequest< SeedResponses >( context< CAcceptNodeAction >().getActionKey(), new CSpecificMediumFilter( context< CAcceptNodeAction >().getMediumPtr() ) ) );
		return discard_event();
	}

	boost::statechart::result react( const common::CContinueEvent & _continueEvent )
	{
		if ( GetTime() - m_time > WaitTime )
		{
			return transit< CCantReachNode >();
		}
		else
		{
			context< CAcceptNodeAction >().setRequest( new common::CContinueReqest<SeedResponses>( _continueEvent.m_keyId, new CSpecificMediumFilter( context< CAcceptNodeAction >().getMediumPtr() ) ) );
			return discard_event();
		}
	}

	boost::statechart::result react( common::CAckPromptResult const & _ackPrompt )
	{
		context< CAcceptNodeAction >().setRequest( new common::CNetworkRoleRequest<SeedResponses>( context< CAcceptNodeAction >().getActionKey(), common::CRole::Seed, new CSpecificMediumFilter( context< CAcceptNodeAction >().getMediumPtr() ) ) );
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
	boost::statechart::custom_reaction< common::CContinueEvent >,
	boost::statechart::custom_reaction< common::CAckPromptResult >,
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
		context< CAcceptNodeAction >().setRequest( new common::CContinueReqest<SeedResponses>( context< CAcceptNodeAction >().getActionKey(), new CSpecificMediumFilter( context< CAcceptNodeAction >().getMediumPtr() ) ) );
	}

	boost::statechart::result react( const common::CContinueEvent & _continueEvent )
	{
		if ( GetTime() - m_time > WaitTime )
		{
			return transit< CCantReachNode >();
		}
		else
		{
			context< CAcceptNodeAction >().setRequest( new common::CContinueReqest<SeedResponses>( _continueEvent.m_keyId, new CSpecificMediumFilter( context< CAcceptNodeAction >().getMediumPtr() ) ) );
			return discard_event();
		}
	}

	boost::statechart::result react( const common::CIntroduceEvent & _introduceEvent )
	{
		uint256 hash = Hash( &_introduceEvent.m_payload.front(), &_introduceEvent.m_payload.back() );

		if ( _introduceEvent.m_key.Verify( hash, _introduceEvent.m_signed ) )
		{
			CSeedNodesManager::getInstance()->setPublicKey( _introduceEvent.m_address, _introduceEvent.m_key );
			context< CAcceptNodeAction >().setRequest( new common::CAckRequest< SeedResponses >( context< CAcceptNodeAction >().getActionKey(), new CSpecificMediumFilter( context< CAcceptNodeAction >().getMediumPtr() ) ) );
			context< CAcceptNodeAction >().setAddress( _introduceEvent.m_address );
		}
		else
		{
			context< CAcceptNodeAction >().setValid( false );
			context< CAcceptNodeAction >().setRequest( 0 );
		}

		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CContinueEvent >,
	boost::statechart::custom_reaction< common::CIntroduceEvent >,
	boost::statechart::transition< common::CAckPromptResult, CDetermineRoleConnected >
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
		context< CAcceptNodeAction >().setRequest( new common::CIdentifyRequest<SeedResponses>( new CSpecificMediumFilter( convertToInt( connectedEvent->m_node ) ), context< CAcceptNodeAction >().getPayload(), context< CAcceptNodeAction >().getActionKey() ) );

	}

	boost::statechart::result react( const common::CContinueEvent & _continueEvent )
	{
		if ( GetTime() - m_time > WaitTime )
		{
			return transit< CCantReachNode >();
		}
		else
		{
			context< CAcceptNodeAction >().setRequest( new common::CContinueReqest<SeedResponses>( _continueEvent.m_keyId, new CSpecificMediumFilter( context< CAcceptNodeAction >().getMediumPtr() ) ) );
			return discard_event();
		}
	}

	boost::statechart::result react( common::CAckEvent const & _ackEvent )
	{
		context< CAcceptNodeAction >().setRequest( new common::CContinueReqest<SeedResponses>( context< CAcceptNodeAction >().getActionKey(), new CSpecificMediumFilter( context< CAcceptNodeAction >().getMediumPtr() ) ) );
		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::transition< common::CIntroduceEvent, CPairIdentifiedConnecting >,
	boost::statechart::custom_reaction< common::CContinueEvent >,
	boost::statechart::custom_reaction< common::CAckEvent >
	> reactions;

	uint64_t m_time;
};

struct CBothUnidentifiedConnected : boost::statechart::state< CBothUnidentifiedConnected, CAcceptNodeAction >
{
	CBothUnidentifiedConnected( my_context ctx ) : my_base( ctx )
	{
		m_time = GetTime();

		context< CAcceptNodeAction >().setRequest( new common::CAckRequest<SeedResponses>( context< CAcceptNodeAction >().getActionKey(), new CSpecificMediumFilter( context< CAcceptNodeAction >().getMediumPtr() ) ) );
	}

	boost::statechart::result react( const common::CContinueEvent & _continueEvent )
	{
		if ( GetTime() - m_time > WaitTime )
		{
			return transit< CCantReachNode >();
		}
		else
		{
			context< CAcceptNodeAction >().setRequest( new common::CContinueReqest<SeedResponses>( _continueEvent.m_keyId, new CSpecificMediumFilter( context< CAcceptNodeAction >().getMediumPtr() ) ) );
			return discard_event();
		}
	}

	boost::statechart::result react( common::CAckPromptResult const & _ackPrompt )
	{
		createIdentifyResponse( context< CAcceptNodeAction >() );
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CContinueEvent >,
	boost::statechart::custom_reaction< common::CAckPromptResult >,
	boost::statechart::transition< common::CAckEvent, CPairIdentifiedConnected >
	> reactions;

	uint64_t m_time;
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
					new common::CConnectToNodeRequest< SeedResponses >( std::string(""), context< CAcceptNodeAction >().getAddress(), new CInternalMediumFilter() ) );

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

		context< CAcceptNodeAction >().setRequest(
					new common::CKnownNetworkInfoRequest< SeedResponses >( context< CAcceptNodeAction >().getActionKey(), common::CKnownNetworkInfo(), new CSpecificMediumFilter( context< CAcceptNodeAction >().getMediumPtr() ) ) );// vicious usage of CKnownNetworkInfoRequest
	}

	boost::statechart::result react( common::CNetworkInfoEvent const & _networkInfo )
	{
		context< CAcceptNodeAction >().setRequest( 0 );

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

	boost::statechart::result react( const common::CContinueEvent & _continueEvent )
	{
		if ( GetTime() - m_time > WaitTime )
		{
			return transit< CCantReachNode >();
		}
		else
		{
			context< CAcceptNodeAction >().setRequest( new common::CContinueReqest< SeedResponses >( _continueEvent.m_keyId, new CSpecificMediumFilter( context< CAcceptNodeAction >().getMediumPtr() ) ) );

			return discard_event();
		}
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CNetworkInfoEvent >,
	boost::statechart::custom_reaction< common::CContinueEvent >
	> reactions;

	uint64_t m_time;
};

struct ConnectedToSeed : boost::statechart::state< ConnectedToSeed, CAcceptNodeAction >
{
	ConnectedToSeed( my_context ctx ) : my_base( ctx )
	{
		context< CAcceptNodeAction >().setRequest( 0 );
	}
};

struct ConnectedToMonitor : boost::statechart::state< ConnectedToMonitor, CAcceptNodeAction >
{
	ConnectedToMonitor( my_context ctx ) : my_base( ctx )
	{
		m_time = GetTime();

		context< CAcceptNodeAction >().setValid( true );

		context< CAcceptNodeAction >().setRequest(
					new common::CKnownNetworkInfoRequest< SeedResponses >( context< CAcceptNodeAction >().getActionKey(), common::CKnownNetworkInfo(), new CSpecificMediumFilter( context< CAcceptNodeAction >().getMediumPtr() ) ) );// vicious usage of CKnownNetworkInfoRequest
	}

	boost::statechart::result react( common::CNetworkInfoEvent const & _networkInfo )
	{
		context< CAcceptNodeAction >().setRequest( 0 );

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

	boost::statechart::result react( const common::CContinueEvent & _continueEvent )
	{
		if ( GetTime() - m_time > WaitTime )
		{
			return transit< CCantReachNode >();
		}
		else
		{
			context< CAcceptNodeAction >().setRequest( new common::CContinueReqest< SeedResponses >( _continueEvent.m_keyId, new CSpecificMediumFilter( context< CAcceptNodeAction >().getMediumPtr() ) ) );

			return discard_event();
		}
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CNetworkInfoEvent >,
	boost::statechart::custom_reaction< common::CContinueEvent >
	> reactions;

	uint64_t m_time;
};


struct CSynchronizing : boost::statechart::simple_state< CSynchronizing, CAcceptNodeAction >
{

};

CAcceptNodeAction::CAcceptNodeAction( uint256 const & _actionKey, std::vector< unsigned char > const & _payload, uintptr_t _mediumPtr )
	: common::CCommunicationAction( _actionKey )
	, m_payload( _payload )
	, m_request( 0 )
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
	common::CRequest< SeedResponses >* request = m_request;
	m_request = 0;
	return request;
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

common::CRequest< SeedResponses > const *
CAcceptNodeAction::getRequest() const
{
	return m_request;
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
