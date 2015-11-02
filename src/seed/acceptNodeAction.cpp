// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "acceptNodeAction.h"
#include "common/setResponseVisitor.h"
#include "common/events.h"
#include "common/authenticationProvider.h"
#include "common/requests.h"

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

extern CAddrDb db;

struct CUnconnected;
struct CBothUnidentifiedConnected;
struct CCantReachNode;
struct CGetNetworkInfo;
struct ConnectedToSeed;
struct CBothUnidentifiedConnecting;
struct CPairIdentifiedConnecting;
struct CDetermineRoleConnecting;
struct CDetermineRoleConnected;

uint64_t const WaitTime = 10000;

common::CRequest *
createIdentifyResponse( 	std::vector<unsigned char> const &_payload, uint256 const & _actionKey,common::CMediumFilter* _medium )
{
	uint256 hash = Hash( &_payload.front(), &_payload.back() );

	std::vector< unsigned char > signedHash;
	common::CAuthenticationProvider::getInstance()->sign( hash, signedHash );

	return new common::CSendIdentifyDataRequest( signedHash, common::CAuthenticationProvider::getInstance()->getMyKey(), _payload, _actionKey, _medium );
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

		context< CAcceptNodeAction >().forgetRequests();
		context< CAcceptNodeAction >().addRequest(
					new common::CConnectToNodeRequest( std::string(""), m_usedAddress, new CMediumClassFilter( common::CMediumKinds::Internal ) ) );
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
			context< CAcceptNodeAction >().forgetRequests();
			context< CAcceptNodeAction >().addRequest(
						new common::CConnectToNodeRequest( std::string(""), m_usedAddress, new CMediumClassFilter( common::CMediumKinds::Internal ) ) );
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

		CSeedNodesManager::getInstance()->addNode( new CSeedNodeMedium( connectedEvent->m_node ) );
		context< CAcceptNodeAction >().forgetRequests();

		context< CAcceptNodeAction >().addRequest(
					createIdentifyResponse(
						context< CAcceptNodeAction >().getPayload(),
						context< CAcceptNodeAction >().getActionKey(),
						new CSpecificMediumFilter( context< CAcceptNodeAction >().getNodePtr() )
						)
					);

		context< CAcceptNodeAction >().addRequest( new common::CTimeEventRequest( WaitTime, new CMediumClassFilter( common::CMediumKinds::Time ) ) );
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

		context< CAcceptNodeAction >().forgetRequests();
		context< CAcceptNodeAction >().addRequest( new common::CTimeEventRequest( WaitTime, new CMediumClassFilter( common::CMediumKinds::Time ) ) );
	}

	boost::statechart::result react( common::CIdentificationResult const & _identificationResult )
	{
		context< CAcceptNodeAction >().setPublicKey( _identificationResult.m_key );
		uint256 hash = Hash( &_identificationResult.m_payload.front(), &_identificationResult.m_payload.back() );

		if ( _identificationResult.m_key.Verify( hash, _identificationResult.m_signed ) )
		{
			CSeedNodesManager::getInstance()->setPublicKey( context< CAcceptNodeAction >().getNodePtr(), _identificationResult.m_key );

			context< CAcceptNodeAction >().forgetRequests();
			context< CAcceptNodeAction >().addRequest(
						new common::CAckRequest(
							  context< CAcceptNodeAction >().getActionKey()
							, _identificationResult.m_id
							, new CSpecificMediumFilter( context< CAcceptNodeAction >().getNodePtr() ) ) );

			context< CAcceptNodeAction >().addRequest(
						new common::CTimeEventRequest(
							  WaitTime
							, new CMediumClassFilter( common::CMediumKinds::Time ) ) );

			context< CAcceptNodeAction >().setAddress( _identificationResult.m_address );
		}
		else
		{
		// something  is  wrong  with  pair react  somehow for  now put 0
			assert( !"message wrongly signed" );
			context< CAcceptNodeAction >().forgetRequests();
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

		context< CAcceptNodeAction >().addRequest(
				new common::CSendMessageRequest(
					common::CPayloadKind::InfoReq
					, common::CInfoRequestData( (int)common::CInfoKind::RoleInfoAsk, std::vector<unsigned char>() )
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

			context< CAcceptNodeAction >().addRequest(
					new common::CSendMessageRequest(
						common::CPayloadKind::RoleInfo
						, common::CNetworkRole( (int)common::CRole::Seed )
						, context< CAcceptNodeAction >().getActionKey()
						, _messageResult.m_message.m_header.m_id
						, new CSpecificMediumFilter( context< CAcceptNodeAction >().getNodePtr() ) ) );
		}
		else if ( orginalMessage.m_header.m_payloadKind == common::CPayloadKind::RoleInfo )
		{
			common::CNetworkRole networkRole;
			common::convertPayload( orginalMessage, networkRole );

			context< CAcceptNodeAction >().forgetRequests();
			context< CAcceptNodeAction >().addRequest(
						new common::CAckRequest(
							  context< CAcceptNodeAction >().getActionKey()
							, _messageResult.m_message.m_header.m_id
							, new CSpecificMediumFilter( context< CAcceptNodeAction >().getNodePtr() ) ) );

			m_role = ( common::CRole::Enum )networkRole.m_role;
		}

		return discard_event();
	}

	boost::statechart::result react( common::CAckEvent const & _ackEvent )
	{
		switch ( m_role )
		{
		case common::CRole::Tracker:
			LogPrintf("accept node action: %p connected to tracker \n", &context< CAcceptNodeAction >() );
			return transit< CGetNetworkInfo >();
		case common::CRole::Seed:
			return transit< ConnectedToSeed >();
		case common::CRole::Monitor:
			LogPrintf("accept node action: %p connected to monitor \n", &context< CAcceptNodeAction >() );
			return transit< CGetNetworkInfo >();
		default:
			break;
		}
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
	common::CRole::Enum m_role;
};

struct CBothUnidentifiedConnected : boost::statechart::state< CBothUnidentifiedConnected, CAcceptNodeAction >
{
	CBothUnidentifiedConnected( my_context ctx ) : my_base( ctx )
	{
		LogPrintf("accept node action: %p both unidentified connected \n", &context< CAcceptNodeAction >() );
	}
	boost::statechart::result react( common::CIdentificationResult const & _identificationResult )
	{
		context< CAcceptNodeAction >().setPublicKey( _identificationResult.m_key );
		uint256 hash = Hash( &_identificationResult.m_payload.front(), &_identificationResult.m_payload.back() );

		if ( _identificationResult.m_key.Verify( hash, _identificationResult.m_signed ) )
		{
			if ( CSeedNodesManager::getInstance()->isKnown( _identificationResult.m_key ) )
			{
				// drop it, already known
				context< CAcceptNodeAction >().forgetRequests();
				return discard_event();
			}

			CSeedNodesManager::getInstance()->setPublicKey( context< CAcceptNodeAction >().getNodePtr(), _identificationResult.m_key );
			context< CAcceptNodeAction >().forgetRequests();
			context< CAcceptNodeAction >().addRequest(
						new common::CAckRequest(
							  context< CAcceptNodeAction >().getActionKey()
							, _identificationResult.m_id
							, new CSpecificMediumFilter( context< CAcceptNodeAction >().getNodePtr() ) ) );

			context< CAcceptNodeAction >().addRequest(
						createIdentifyResponse(
							_identificationResult.m_payload,
							context< CAcceptNodeAction >().getActionKey(),
							new CSpecificMediumFilter( context< CAcceptNodeAction >().getNodePtr() )
							)
						);

			context< CAcceptNodeAction >().addRequest( new common::CTimeEventRequest( WaitTime, new CMediumClassFilter( common::CMediumKinds::Time ) ) );

			context< CAcceptNodeAction >().setAddress( _identificationResult.m_address );
		}
		else
		{
			// something  is  wrong  with  pair react  somehow for  now put 0
			context< CAcceptNodeAction >().forgetRequests();
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
		context< CAcceptNodeAction >().forgetRequests();
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

			context< CAcceptNodeAction >().addRequest(
					new common::CSendMessageRequest(
						common::CPayloadKind::RoleInfo
						, common::CNetworkRole( (int)common::CRole::Seed )
						, context< CAcceptNodeAction >().getActionKey()
						, _messageResult.m_message.m_header.m_id
						, new CSpecificMediumFilter( context< CAcceptNodeAction >().getNodePtr() ) ) );
		}
		else if ( orginalMessage.m_header.m_payloadKind == common::CPayloadKind::RoleInfo )
		{
			common::CNetworkRole networkRole;
			common::convertPayload( orginalMessage, networkRole );

			context< CAcceptNodeAction >().forgetRequests();
			context< CAcceptNodeAction >().addRequest(
						new common::CAckRequest(
							  context< CAcceptNodeAction >().getActionKey()
							, _messageResult.m_message.m_header.m_id
							, new CSpecificMediumFilter( context< CAcceptNodeAction >().getNodePtr() ) ) );

			switch ( networkRole.m_role )
			{
			case common::CRole::Tracker:
				LogPrintf("accept node action: %p connected to tracker \n", &context< CAcceptNodeAction >() );
				db.Add( context< CAcceptNodeAction >().getAddress() );
				return transit< CGetNetworkInfo >();
			case common::CRole::Seed:
				return transit< ConnectedToSeed >();
			case common::CRole::Monitor:
				LogPrintf("accept node action: %p connected to monitor \n", &context< CAcceptNodeAction >() );
				db.Add( context< CAcceptNodeAction >().getAddress() );
				return transit< CGetNetworkInfo >();
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
		context< CAcceptNodeAction >().forgetRequests();

		context< CAcceptNodeAction >().addRequest(
				new common::CSendMessageRequest(
					common::CPayloadKind::InfoReq
					, common::CInfoRequestData( (int)common::CInfoKind::RoleInfoAsk, std::vector<unsigned char>() )
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
		context< CAcceptNodeAction >().forgetRequests();
		context< CAcceptNodeAction >().setValid( false );
		context< CAcceptNodeAction >().setExit();
	}
};

struct CGetNetworkInfo : boost::statechart::state< CGetNetworkInfo, CAcceptNodeAction >
{
	CGetNetworkInfo( my_context ctx ) : my_base( ctx )
	{
		context< CAcceptNodeAction >().setValid( true );

		context< CAcceptNodeAction >().forgetRequests();

		context< CAcceptNodeAction >().addRequest(
				new common::CSendMessageRequest(
					common::CPayloadKind::InfoReq
					, common::CInfoRequestData( (int)common::CInfoKind::NetworkInfoAsk, std::vector<unsigned char>() )
					, context< CAcceptNodeAction >().getActionKey()
					, new CSpecificMediumFilter( context< CAcceptNodeAction >().getNodePtr() ) ) );

		context< CAcceptNodeAction >().addRequest( new common::CTimeEventRequest( WaitTime, new CMediumClassFilter( common::CMediumKinds::Time ) ) );
	}

	boost::statechart::result react( common::CMessageResult const & _messageResult )
	{
		common::CMessage orginalMessage;
		if ( !common::CommunicationProtocol::unwindMessage( _messageResult.m_message, orginalMessage, GetTime(), context< CAcceptNodeAction >().getPublicKey() ) )
			assert( !"service it somehow" );

		if ( orginalMessage.m_header.m_payloadKind == common::CPayloadKind::NetworkInfo )
		{
			common::CKnownNetworkInfo knownNetworkInfo;

			common::convertPayload( orginalMessage, knownNetworkInfo );

			context< CAcceptNodeAction >().forgetRequests();

			context< CAcceptNodeAction >().addRequest(
						new common::CAckRequest(
							  context< CAcceptNodeAction >().getActionKey()
							, _messageResult.m_message.m_header.m_id
							, new CSpecificMediumFilter( context< CAcceptNodeAction >().getNodePtr() ) ) );

			BOOST_FOREACH( common::CValidNodeInfo validNodeInfo, knownNetworkInfo.m_trackersInfo )
			{
				db.Add( validNodeInfo.m_address );
			}

			BOOST_FOREACH( common::CValidNodeInfo validNodeInfo, knownNetworkInfo.m_monitorsInfo )
			{
				db.Add( validNodeInfo.m_address );
			}
		}

		return discard_event();
	}

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		context< CAcceptNodeAction >().setExit();
		return discard_event();
	}

	boost::statechart::result react( common::CAckEvent const & _ackEvent )
	{
		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CMessageResult >,
	boost::statechart::custom_reaction< common::CTimeEvent >,
	boost::statechart::custom_reaction< common::CAckEvent >
	> reactions;
};

struct ConnectedToSeed : boost::statechart::state< ConnectedToSeed, CAcceptNodeAction >
{
	ConnectedToSeed( my_context ctx ) : my_base( ctx )
	{
		LogPrintf("accept node action: %p connected to seed \n", &context< CAcceptNodeAction >() );
	}
};

CAcceptNodeAction::CAcceptNodeAction( uint256 const & _actionKey, uintptr_t _nodePtr, CServiceResult & _service )
	: common::CAction( _actionKey )
	, m_nodePtr( _nodePtr )
	, m_passive( true )
	, m_valid( false )
	, m_service( _service )
{
	initiate();
}

CAcceptNodeAction::CAcceptNodeAction( CServiceResult & _service )
	: m_passive( false )
	, m_valid( false )
	, m_service( _service )
{
	for ( unsigned int i = 0; i < ms_randomPayloadLenght; i++ )
	{
		m_payload.push_back( insecure_rand() % 256 );
	}
	m_nodeAddress = CAddress(m_service.service);
	// !! use  default  no  matter  what  is  there
	// it  warks   but  is  it acctually correct??
	m_nodeAddress.SetPort( common::dimsParams().GetDefaultPort() );

	initiate();
}

void
CAcceptNodeAction::accept( common::CSetResponseVisitor & _visitor )
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
