// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "acceptNodeAction.h"
#include "common/setResponseVisitor.h"
#include "common/commonEvents.h"
#include "common/authenticationProvider.h"
#include "common/commonRequests.h"

#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/state.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/custom_reaction.hpp>

#include "seed/acceptNodeAction.h"
#include "seed/seedNodesManager.h"
#include "seed/seedNodeMedium.h"
#include "seed/pingAction.h"
#include "seed/seedDb.h"
#include "seed/seedFilter.h"

// ugly as hell, refactor as soon as possible
namespace seed
{
boost::mutex mutex;
std::map< std::string, bool > m_result;

void addResult( std::string const & _key, bool _value )
{
	boost::lock_guard<boost::mutex> lock( mutex );
	m_result.insert( std::make_pair( _key, _value ) );
}

bool
getResult( std::string const & _key, bool & _value )
{
	boost::lock_guard<boost::mutex> lock( mutex );
	std::map< std::string, bool >::const_iterator iterator = m_result.find( _key );

	if ( iterator == m_result.end() )
		return false;
	_value = iterator->second;

	return true;
}

extern CAddrDb db;

struct CUnconnected;
struct CBothUnidentifiedConnected;
struct CCantReachNode;
struct ConnectedToTracker;
struct ConnectedToSeed;
struct ConnectedToMonitor;
struct CBothUnidentifiedConnecting;
struct CPairIdentifiedConnecting;
struct CDetermineRoleConnecting;
struct CDetermineRoleConnected;

uint64_t const WaitTime = 10000;

common::CRequest< common::CSeedTypes > *
createIdentifyResponse( 	std::vector<unsigned char> const &_payload, uint256 const & _actionKey,common::CSeedMediumFilter* _medium )
{
	uint256 hash = Hash( &_payload.front(), &_payload.back() );

	std::vector< unsigned char > signedHash;
	common::CAuthenticationProvider::getInstance()->sign( hash, signedHash );

	return new common::CSendIdentifyDataRequest< common::CSeedTypes >( signedHash, common::CAuthenticationProvider::getInstance()->getMyKey(), _payload, _actionKey, _medium );
}


struct CUninitiated : boost::statechart::simple_state< CUninitiated, CAcceptNodeAction >
{
	typedef boost::mpl::list<
	boost::statechart::transition< common::CSwitchToConnectingEvent, CUnconnected >,
	boost::statechart::transition< common::CSwitchToConnectedEvent, CBothUnidentifiedConnected >
	> reactions;

};

struct CUnconnected : boost::statechart::state< CUnconnected, CAcceptNodeAction >
{
	CUnconnected( my_context ctx )
		: my_base( ctx )
		, m_default(false)
	{
		LogPrintf("accept node action: %p unconnected state \n", &context< CAcceptNodeAction >() );
		m_usedAddress = context< CAcceptNodeAction >().getAddress();

		context< CAcceptNodeAction >().dropRequests();
		context< CAcceptNodeAction >().addRequests(
					new common::CConnectToNodeRequest< common::CSeedTypes >( std::string(""), m_usedAddress, new CMediumClassFilter( common::CMediumKinds::Internal ) ) );
	}

	boost::statechart::result react( common::CCantReachNode const & _cantReach )
	{
		if ( m_default )
		{
			return transit< CCantReachNode >();
		}
		else
		{
			m_default = true;
			m_usedAddress.SetPort( common::dimsParams().GetDefaultPort() );
			context< CAcceptNodeAction >().dropRequests();
			context< CAcceptNodeAction >().addRequests(
						new common::CConnectToNodeRequest< common::CSeedTypes >( std::string(""), m_usedAddress, new CMediumClassFilter( common::CMediumKinds::Internal ) ) );
		}
		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::transition< common::CNodeConnectedEvent, CBothUnidentifiedConnecting >,
	boost::statechart::custom_reaction< common::CCantReachNode >
	> reactions;

	CAddress m_usedAddress;
	bool m_default;
};

struct CBothUnidentifiedConnecting : boost::statechart::state< CBothUnidentifiedConnecting, CAcceptNodeAction >
{
	CBothUnidentifiedConnecting( my_context ctx ) : my_base( ctx )
	{
		LogPrintf("accept node action: %p both unidentified connecting \n", &context< CAcceptNodeAction >() );
		common::CNodeConnectedEvent const* connectedEvent = dynamic_cast< common::CNodeConnectedEvent const* >( simple_state::triggering_event() );
		context< CAcceptNodeAction >().setNodePtr( convertToInt( connectedEvent->m_node ) );

		if ( !CPingAction::isPinged( convertToInt( connectedEvent->m_node ) ) )
		{
			CPingAction * pingAction= new CPingAction( convertToInt( connectedEvent->m_node ) );

			pingAction->process_event( common::CStartPingEvent() );

			common::CActionHandler< common::CSeedTypes >::getInstance()->executeAction( pingAction );
		}

		CSeedNodesManager::getInstance()->addNode( new CSeedNodeMedium( connectedEvent->m_node ) );
		context< CAcceptNodeAction >().dropRequests();

		context< CAcceptNodeAction >().addRequests(
					createIdentifyResponse(
						context< CAcceptNodeAction >().getPayload(),
						context< CAcceptNodeAction >().getActionKey(),
						new CSpecificMediumFilter( context< CAcceptNodeAction >().getNodePtr() )
						)
					);

		context< CAcceptNodeAction >().addRequests( new common::CTimeEventRequest< common::CSeedTypes >( WaitTime, new CMediumClassFilter( common::CMediumKinds::Time ) ) );
	}

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		return transit< CCantReachNode >();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CTimeEvent >,
	boost::statechart::transition< common::CAckEvent, CPairIdentifiedConnecting >
	> reactions;
};

struct CPairIdentifiedConnecting : boost::statechart::state< CPairIdentifiedConnecting, CAcceptNodeAction >
{
	CPairIdentifiedConnecting( my_context ctx ) : my_base( ctx )
	{
		LogPrintf("accept node action: %p pair identified connecting \n", &context< CAcceptNodeAction >() );

		context< CAcceptNodeAction >().addRequests( new common::CTimeEventRequest< common::CSeedTypes >( WaitTime, new CMediumClassFilter( common::CMediumKinds::Time ) ) );
	}

	boost::statechart::result react( common::CIdentificationResult const & _identificationResult )
	{
		uint256 hash = Hash( &_identificationResult.m_payload.front(), &_identificationResult.m_payload.back() );

		if ( _identificationResult.m_key.Verify( hash, _identificationResult.m_signed ) )
		{
			CSeedNodesManager::getInstance()->setPublicKey( context< CAcceptNodeAction >().getNodePtr(), _identificationResult.m_key );

			context< CAcceptNodeAction >().dropRequests();
			context< CAcceptNodeAction >().addRequests(
						new common::CAckRequest< common::CSeedTypes >(
							  context< CAcceptNodeAction >().getActionKey()
							, _identificationResult.m_id
							, new CSpecificMediumFilter( context< CAcceptNodeAction >().getNodePtr() ) ) );

			context< CAcceptNodeAction >().addRequests(
						new common::CTimeEventRequest< common::CSeedTypes >(
							  WaitTime
							, new CMediumClassFilter( common::CMediumKinds::Time ) ) );

			context< CAcceptNodeAction >().setAddress( _identificationResult.m_address );
		}
		else
		{
		// something  is  wrong  with  pair react  somehow for  now put 0
			assert( !"message wrongly signed" );
			context< CAcceptNodeAction >().dropRequests();
		}
		return transit< CDetermineRoleConnecting >();
	}

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		return transit< CCantReachNode >();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CTimeEvent >,
	boost::statechart::custom_reaction< common::CIdentificationResult >
	> reactions;
};

struct CDetermineRoleConnecting : boost::statechart::state< CDetermineRoleConnecting, CAcceptNodeAction >
{
	CDetermineRoleConnecting( my_context ctx ) : my_base( ctx )
	{
		LogPrintf("accept node action: %p determine role connecting \n", &context< CAcceptNodeAction >() );

		context< CAcceptNodeAction >().addRequests(
					  new common::CInfoAskRequest< common::CSeedTypes >(
						  common::CInfoKind::RoleInfoAsk
						, context< CAcceptNodeAction >().getActionKey()
						, new CSpecificMediumFilter( context< CAcceptNodeAction >().getNodePtr() ) ) );

	}

	boost::statechart::result react( common::CMessageResult const & _messageResult )
	{
		common::CMessage orginalMessage;
		if ( !common::CommunicationProtocol::unwindMessage( _messageResult.m_message, orginalMessage, GetTime(), context< CAcceptNodeAction >().getPublicKey() ) )
			assert( !"service it somehow" );

		if ( orginalMessage.m_header.m_payloadKind == common::CPayloadKind::InfoReq )
		{
			common::CInfoRequestData infoRequest;

			common::convertPayload( orginalMessage, infoRequest );

			assert( infoRequest.m_kind == common::CInfoKind::RoleInfoAsk );

			context< CAcceptNodeAction >().addRequests(
						new common::CNetworkRoleRequest< common::CSeedTypes >(
							  common::CRole::Seed
							, context< CAcceptNodeAction >().getActionKey()
							, infoRequest.m_id
							, new CSpecificMediumFilter( context< CAcceptNodeAction >().getNodePtr() ) ) );
		}
		else if ( orginalMessage.m_header.m_payloadKind == common::CPayloadKind::RoleInfo )
		{
			common::CNetworkRole networkRole;
			common::convertPayload( orginalMessage, networkRole );

			context< CAcceptNodeAction >().dropRequests();
			context< CAcceptNodeAction >().addRequests(
						new common::CAckRequest< common::CSeedTypes >(
							  context< CAcceptNodeAction >().getActionKey()
							, _messageResult.m_message.m_header.m_id
							, new CSpecificMediumFilter( context< CAcceptNodeAction >().getNodePtr() ) ) );

			switch ( networkRole.m_role )
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

		return discard_event();
	}

	boost::statechart::result react( common::CAckEvent const & _ackEvent )
	{

		return discard_event();
	}

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		return transit< CCantReachNode >();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CTimeEvent >,
	boost::statechart::custom_reaction< common::CMessageResult >,
	boost::statechart::custom_reaction< common::CAckEvent >
	> reactions;

	uint64_t m_time;
};

struct CBothUnidentifiedConnected : boost::statechart::state< CBothUnidentifiedConnected, CAcceptNodeAction >
{
	CBothUnidentifiedConnected( my_context ctx ) : my_base( ctx )
	{
		LogPrintf("accept node action: %p both unidentified connected \n", &context< CAcceptNodeAction >() );
	}
	boost::statechart::result react( common::CIdentificationResult const & _identificationResult )
	{
		uint256 hash = Hash( &_identificationResult.m_payload.front(), &_identificationResult.m_payload.back() );

		if ( _identificationResult.m_key.Verify( hash, _identificationResult.m_signed ) )
		{
			if ( CSeedNodesManager::getInstance()->isKnown( _identificationResult.m_key ) )
			{
				// drop it, already known
				context< CAcceptNodeAction >().dropRequests();
				return discard_event();
			}

			CSeedNodesManager::getInstance()->setPublicKey( context< CAcceptNodeAction >().getNodePtr(), _identificationResult.m_key );
			context< CAcceptNodeAction >().dropRequests();
			context< CAcceptNodeAction >().addRequests(
						new common::CAckRequest< common::CSeedTypes >(
							  context< CAcceptNodeAction >().getActionKey()
							, _identificationResult.m_id
							, new CSpecificMediumFilter( context< CAcceptNodeAction >().getNodePtr() ) ) );

			context< CAcceptNodeAction >().addRequests(
						createIdentifyResponse(
							_identificationResult.m_payload,
							context< CAcceptNodeAction >().getActionKey(),
							new CSpecificMediumFilter( context< CAcceptNodeAction >().getNodePtr() )
							)
						);

			context< CAcceptNodeAction >().addRequests( new common::CTimeEventRequest< common::CSeedTypes >( WaitTime, new CMediumClassFilter( common::CMediumKinds::Time ) ) );

			context< CAcceptNodeAction >().setAddress( _identificationResult.m_address );
		}
		else
		{
			// something  is  wrong  with  pair react  somehow for  now put 0
			context< CAcceptNodeAction >().dropRequests();
		}
		return discard_event();
	}

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		return transit< CCantReachNode >();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CTimeEvent >,
	boost::statechart::custom_reaction< common::CIdentificationResult >,
	boost::statechart::transition< common::CAckEvent, CDetermineRoleConnected >
	> reactions;
};

struct CDetermineRoleConnected : boost::statechart::state< CDetermineRoleConnected, CAcceptNodeAction >
{
	CDetermineRoleConnected( my_context ctx ) : my_base( ctx )
	{
		LogPrintf("accept node action: %p determine role connected \n", &context< CAcceptNodeAction >() );
	}

	boost::statechart::result react( common::CMessageResult const & _messageResult )
	{
		common::CMessage orginalMessage;
		if ( !common::CommunicationProtocol::unwindMessage( _messageResult.m_message, orginalMessage, GetTime(), context< CAcceptNodeAction >().getPublicKey() ) )
			assert( !"service it somehow" );

		if ( orginalMessage.m_header.m_payloadKind == common::CPayloadKind::InfoReq )
		{
			common::CInfoRequestData infoRequest;

			common::convertPayload( orginalMessage, infoRequest );

			assert( infoRequest.m_kind == common::CInfoKind::RoleInfoAsk );

			context< CAcceptNodeAction >().addRequests(
						new common::CNetworkRoleRequest< common::CSeedTypes >(
							  common::CRole::Seed
							, context< CAcceptNodeAction >().getActionKey()
							, infoRequest.m_id
							, new CSpecificMediumFilter( context< CAcceptNodeAction >().getNodePtr() ) ) );
		}
		else if ( orginalMessage.m_header.m_payloadKind == common::CPayloadKind::RoleInfo )
		{
			common::CNetworkRole networkRole;
			common::convertPayload( orginalMessage, networkRole );

			context< CAcceptNodeAction >().dropRequests();
			context< CAcceptNodeAction >().addRequests(
						new common::CAckRequest< common::CSeedTypes >(
							  context< CAcceptNodeAction >().getActionKey()
							, _messageResult.m_message.m_header.m_id
							, new CSpecificMediumFilter( context< CAcceptNodeAction >().getNodePtr() ) ) );

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

		}

		return discard_event();
	}

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		return transit< CCantReachNode >();
	}

	boost::statechart::result react( common::CAckEvent const & _ackEvent )
	{
		context< CAcceptNodeAction >().dropRequests();

		context< CAcceptNodeAction >().addRequests(
					  new common::CInfoAskRequest< common::CSeedTypes >(
						  common::CInfoKind::RoleInfoAsk
						, context< CAcceptNodeAction >().getActionKey()
						, new CSpecificMediumFilter( context< CAcceptNodeAction >().getNodePtr() ) ) );

		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CMessageResult >,
	boost::statechart::custom_reaction< common::CTimeEvent >,
	boost::statechart::custom_reaction< common::CAckEvent >
	> reactions;

	int m_role;
	uint64_t m_time;
};

struct CCantReachNode : boost::statechart::state< CCantReachNode, CAcceptNodeAction >
{
	CCantReachNode( my_context ctx ) : my_base( ctx )
	{
		LogPrintf("accept node action: %p can't reach node \n", &context< CAcceptNodeAction >() );

		context< CAcceptNodeAction >().setValid( false );
		context< CAcceptNodeAction >().dropRequests();
	}
};

struct ConnectedToTracker : boost::statechart::state< ConnectedToTracker, CAcceptNodeAction >
{
	ConnectedToTracker( my_context ctx ) : my_base( ctx )
	{
		LogPrintf("accept node action: %p connected to tracker \n", &context< CAcceptNodeAction >() );

		m_time = GetTime();

		context< CAcceptNodeAction >().setValid( true );

		context< CAcceptNodeAction >().addRequests(
					new common::CKnownNetworkInfoRequest< common::CSeedTypes >( context< CAcceptNodeAction >().getActionKey(), common::CKnownNetworkInfo(), new CSpecificMediumFilter( context< CAcceptNodeAction >().getNodePtr() ) ) );// vicious usage of CKnownNetworkInfoRequest
		context< CAcceptNodeAction >().addRequests( new common::CTimeEventRequest< common::CSeedTypes >( 20000, new CMediumClassFilter( common::CMediumKinds::Time ) ) );
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

	boost::statechart::result react( common::CAckEvent const & _ackEvent )
	{
		context< CAcceptNodeAction >().dropRequests();
		return discard_event();
	}

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		context< CAcceptNodeAction >().dropRequests();
		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CTimeEvent >,
	boost::statechart::custom_reaction< common::CNetworkInfoEvent >,
	boost::statechart::custom_reaction< common::CAckEvent >
	> reactions;

	uint64_t m_time;
};

struct ConnectedToSeed : boost::statechart::state< ConnectedToSeed, CAcceptNodeAction >
{
	ConnectedToSeed( my_context ctx ) : my_base( ctx )
	{
		LogPrintf("accept node action: %p connected to seed \n", &context< CAcceptNodeAction >() );
	}
};


struct ConnectedToMonitor : boost::statechart::state< ConnectedToMonitor, CAcceptNodeAction >
{
	ConnectedToMonitor( my_context ctx ) : my_base( ctx )
	{
		LogPrintf("accept node action: %p connected to monitor \n", &context< CAcceptNodeAction >() );

		context< CAcceptNodeAction >().setValid( true );

		context< CAcceptNodeAction >().addRequests(
					new common::CKnownNetworkInfoRequest< common::CSeedTypes >( context< CAcceptNodeAction >().getActionKey(), common::CKnownNetworkInfo(), new CSpecificMediumFilter( context< CAcceptNodeAction >().getNodePtr() ) ) );// vicious usage of CKnownNetworkInfoRequest
		context< CAcceptNodeAction >().addRequests( new common::CTimeEventRequest< common::CSeedTypes >( 20000, new CMediumClassFilter( common::CMediumKinds::Time ) ) );
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


CAcceptNodeAction::CAcceptNodeAction( uint256 const & _actionKey, uintptr_t _nodePtr )
	: common::CAction< common::CSeedTypes >( _actionKey )
	, common::CCommunicationAction( _actionKey )
	, m_passive( true )
	, m_nodePtr( _nodePtr )
	, m_valid( false )
{
	initiate();
}

CAcceptNodeAction::CAcceptNodeAction( CAddress const & _nodeAddress )
	: CCommunicationAction( getActionKey() )
	, m_nodeAddress( _nodeAddress )
	, m_passive( false )
	, m_valid( false )
{
	for ( unsigned int i = 0; i < ms_randomPayloadLenght; i++ )
	{
		m_payload.push_back( insecure_rand() % 256 );
	}
	initiate();
}

void
CAcceptNodeAction::accept( common::CSetResponseVisitor< common::CSeedTypes > & _visitor )
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
CAcceptNodeAction::getNodePtr() const
{
	return m_nodePtr;
}

void
CAcceptNodeAction::setNodePtr( uintptr_t _nodePtr )
{
	m_nodePtr = _nodePtr;
}

CPubKey
CAcceptNodeAction::getPublicKey() const
{
	return m_key;
}

void
CAcceptNodeAction::setPublicKey( CPubKey const & _pubKey )
{
	m_key = _pubKey;
}

}
