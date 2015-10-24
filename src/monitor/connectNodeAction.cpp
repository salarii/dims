// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "connectNodeAction.h"
#include "common/setResponseVisitor.h"
#include "common/events.h"
#include "common/authenticationProvider.h"
#include "common/requests.h"

#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/state.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/custom_reaction.hpp>

#include "monitor/filters.h"
#include "monitor/monitorRequests.h"
#include "monitor/reputationTracer.h"
#include "monitor/controller.h"

namespace monitor
{

unsigned int const LoopTime = 20000;//milices

struct CMonitorUnconnected;
struct CMonitorBothUnidentifiedConnected;
struct CMonitorConnectedToSeed;
struct CMonitorBothUnidentifiedConnecting;
struct CMonitorCantReachNode;
struct CGetNetworkInfo;
struct CMonitorPairIdentifiedConnecting;
struct CMonitorDetermineRoleConnecting;
struct CMonitorDetermineRoleConnected;

common::CRequest *
createIdentifyResponse( 	std::vector<unsigned char> const &_payload, uint256 const & _actionKey,common::CMediumFilter* _medium )
{
	uint256 hash = Hash( &_payload.front(), &_payload.back() );

	std::vector< unsigned char > signedHash;
	common::CAuthenticationProvider::getInstance()->sign( hash, signedHash );

	return new common::CSendIdentifyDataRequest( signedHash, common::CAuthenticationProvider::getInstance()->getMyKey(), _payload, _actionKey, _medium );
}

struct CMonitorConnectNodeActionUninitiated : boost::statechart::simple_state< CMonitorConnectNodeActionUninitiated, CConnectNodeAction >
{
	typedef boost::mpl::list<
	boost::statechart::transition< common::CSwitchToConnectingEvent, CMonitorUnconnected >,
	boost::statechart::transition< common::CSwitchToConnectedEvent, CMonitorBothUnidentifiedConnected >
	> reactions;

};

struct CMonitorUnconnected : boost::statechart::state< CMonitorUnconnected, CConnectNodeAction >
{
	CMonitorUnconnected( my_context ctx ) : my_base( ctx )
	{
		LogPrintf("connect node action: %p monitor unconnected \n", &context< CConnectNodeAction >() );

		context< CConnectNodeAction >().forgetRequests();
		context< CConnectNodeAction >().addRequest(
					new CConnectToNodeRequest( "", context< CConnectNodeAction >().getServiceAddress() ) );
	}

	typedef boost::mpl::list<
	boost::statechart::transition< common::CNodeConnectedEvent, CMonitorBothUnidentifiedConnecting >,
	boost::statechart::transition< common::CCantReachNode, CMonitorCantReachNode >
	> reactions;
};

struct CMonitorBothUnidentifiedConnecting : boost::statechart::state< CMonitorBothUnidentifiedConnecting, CConnectNodeAction >
{
	CMonitorBothUnidentifiedConnecting( my_context ctx ) : my_base( ctx )
	{
		LogPrintf("connect node action: %p both unidentified connecting \n", &context< CConnectNodeAction >() );

		common::CNodeConnectedEvent const* connectedEvent = dynamic_cast< common::CNodeConnectedEvent const* >( simple_state::triggering_event() );
		context< CConnectNodeAction >().setNodePtr( convertToInt( connectedEvent->m_node ) );
		// looks funny that  I set it in this  state, but let  it  be
		context< CConnectNodeAction >().setServiceAddress( connectedEvent->m_node->addr);

		CReputationTracker::getInstance()->addNode( new common::CNodeMedium( connectedEvent->m_node ) );

		context< CConnectNodeAction >().forgetRequests();
		context< CConnectNodeAction >().addRequest(
					createIdentifyResponse(
						context< CConnectNodeAction >().getPayload(),
						context< CConnectNodeAction >().getActionKey(),
						new CSpecificMediumFilter( context< CConnectNodeAction >().getNodePtr() )
						)
					);

		context< CConnectNodeAction >().addRequest(
					new common::CTimeEventRequest(
						LoopTime
						, new CMediumClassFilter( common::CMediumKinds::Time ) ) );
	}

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		return transit< CMonitorCantReachNode >();
	}

	typedef boost::mpl::list<
	boost::statechart::transition< common::CAckEvent, CMonitorPairIdentifiedConnecting >,
	boost::statechart::custom_reaction< common::CTimeEvent >
	> reactions;
};


struct CMonitorPairIdentifiedConnecting : boost::statechart::state< CMonitorPairIdentifiedConnecting, CConnectNodeAction >
{
	CMonitorPairIdentifiedConnecting( my_context ctx ) : my_base( ctx )
	{
		LogPrintf("connect node action: %p pair identified connecting \n", &context< CConnectNodeAction >() );

		context< CConnectNodeAction >().addRequest(
					new common::CTimeEventRequest(
						LoopTime
						, new CMediumClassFilter( common::CMediumKinds::Time ) ) );
	}

	boost::statechart::result react( common::CIdentificationResult const & _identificationResult )
	{
		uint256 hash = Hash( &_identificationResult.m_payload.front(), &_identificationResult.m_payload.back() );

		if ( _identificationResult.m_key.Verify( hash, _identificationResult.m_signed ) )
		{
			context< CConnectNodeAction >().setPublicKey( _identificationResult.m_key );
			CReputationTracker::getInstance()->setPublicKey( context< CConnectNodeAction >().getServiceAddress(), _identificationResult.m_key );

			context< CConnectNodeAction >().forgetRequests();

			context< CConnectNodeAction >().addRequest(
						new common::CAckRequest(
							  context< CConnectNodeAction >().getActionKey()
							, _identificationResult.m_id
							, new CSpecificMediumFilter( context< CConnectNodeAction >().getNodePtr() ) ) );

		}
		else
		{
			// something  is  wrong  with  pair react  somehow for  now put 0
			context< CConnectNodeAction >().forgetRequests();
		}
		return transit< CMonitorDetermineRoleConnecting >();
	}

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		return transit< CMonitorCantReachNode >();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CTimeEvent >,
	boost::statechart::custom_reaction< common::CIdentificationResult >
	> reactions;
};

struct CMonitorDetermineRoleConnecting : boost::statechart::state< CMonitorDetermineRoleConnecting, CConnectNodeAction >
{
	CMonitorDetermineRoleConnecting( my_context ctx ) : my_base( ctx )
	{
		LogPrintf("connect node action: %p determine role connecting \n", &context< CConnectNodeAction >() );

		common::CSendMessageRequest * request =
				new common::CSendMessageRequest(
					common::CPayloadKind::InfoReq
					, context< CConnectNodeAction >().getActionKey()
					, new CSpecificMediumFilter( context< CConnectNodeAction >().getNodePtr() ) );

		request->addPayload( common::CInfoKind::RoleInfoAsk );

		context< CConnectNodeAction >().addRequest( request );

	}

	boost::statechart::result react( common::CMessageResult const & _messageResult )
	{
		common::CMessage orginalMessage;
		if ( !common::CommunicationProtocol::unwindMessage( _messageResult.m_message, orginalMessage, GetTime(), context< CConnectNodeAction >().getPublicKey() ) )
			assert( !"service it somehow" );

		if ( orginalMessage.m_header.m_payloadKind == common::CPayloadKind::InfoReq )
		{
			common::CInfoRequestData infoRequest;

			common::convertPayload( orginalMessage, infoRequest );

			assert( infoRequest.m_kind == common::CInfoKind::RoleInfoAsk );

			common::CSendMessageRequest * request =
					new common::CSendMessageRequest(
						common::CPayloadKind::RoleInfo
						, context< CConnectNodeAction >().getActionKey()
						, _messageResult.m_message.m_header.m_id
						, new CSpecificMediumFilter( context< CConnectNodeAction >().getNodePtr() ) );

			request->addPayload(
						common::CNetworkRole( (int)common::CRole::Monitor ) );

			context< CConnectNodeAction >().addRequest( request );

		}
		else if ( orginalMessage.m_header.m_payloadKind == common::CPayloadKind::RoleInfo )
		{
			common::CNetworkRole networkRole;
			common::convertPayload( orginalMessage, networkRole );

			context< CConnectNodeAction >().forgetRequests();
			context< CConnectNodeAction >().addRequest(
						new common::CAckRequest(
							context< CConnectNodeAction >().getActionKey()
							, _messageResult.m_message.m_header.m_id
							, new CSpecificMediumFilter( context< CConnectNodeAction >().getNodePtr() ) ) );

			context< CConnectNodeAction >().setRole( ( common::CRole::Enum )networkRole.m_role );
		}
		return discard_event();
	}

	boost::statechart::result react( common::CAckEvent const & _ackEvent )
	{

		switch ( context< CConnectNodeAction >().getRole() )
		{
		case common::CRole::Tracker:
			LogPrintf("connect node action: %p connected to tracker \n", &context< CConnectNodeAction >() );
			return transit< CGetNetworkInfo >();
		case common::CRole::Seed:
			return transit< CMonitorConnectedToSeed >();
		case common::CRole::Monitor:
			LogPrintf("connect node action: %p connected to monitor \n", &context< CConnectNodeAction >() );
			return transit< CGetNetworkInfo >();
		default:
			break;
		}
		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CMessageResult >,
	boost::statechart::custom_reaction< common::CAckEvent >
	> reactions;
};

struct CMonitorBothUnidentifiedConnected : boost::statechart::state< CMonitorBothUnidentifiedConnected, CConnectNodeAction >
{
	CMonitorBothUnidentifiedConnected( my_context ctx ) : my_base( ctx )
	{
		LogPrintf("connect node action: %p both unidentified connected \n", &context< CConnectNodeAction >() );
	}

	boost::statechart::result react( common::CIdentificationResult const & _identificationResult )
	{
		uint256 hash = Hash( &_identificationResult.m_payload.front(), &_identificationResult.m_payload.back() );

		if ( _identificationResult.m_key.Verify( hash, _identificationResult.m_signed ) )
		{
			context< CConnectNodeAction >().setPublicKey( _identificationResult.m_key );

			context< CConnectNodeAction >().setServiceAddress( _identificationResult.m_address );

			CReputationTracker::getInstance()->setPublicKey( context< CConnectNodeAction >().getServiceAddress(), _identificationResult.m_key );

			context< CConnectNodeAction >().forgetRequests();

			context< CConnectNodeAction >().addRequest(
						new common::CAckRequest(
							  context< CConnectNodeAction >().getActionKey()
							, _identificationResult.m_id
							, new CSpecificMediumFilter( context< CConnectNodeAction >().getNodePtr() ) ) );

			context< CConnectNodeAction >().addRequest(
						createIdentifyResponse(
							_identificationResult.m_payload,
							context< CConnectNodeAction >().getActionKey(),
							new CSpecificMediumFilter( context< CConnectNodeAction >().getNodePtr() )
							)
						);
		}
		else
		{
			assert( !"problem" );
			// something  is  wrong  with  pair react  somehow for  now put 0
			context< CConnectNodeAction >().forgetRequests();
		}
		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CIdentificationResult >,
	boost::statechart::transition< common::CAckEvent, CMonitorDetermineRoleConnected >
	> reactions;
};

struct CMonitorDetermineRoleConnected : boost::statechart::state< CMonitorDetermineRoleConnected, CConnectNodeAction >
{
	CMonitorDetermineRoleConnected( my_context ctx ) : my_base( ctx )
	{
		LogPrintf("connect node action: %p determine role connected \n", &context< CConnectNodeAction >() );
	}

	boost::statechart::result react( common::CMessageResult const & _messageResult )
	{
		common::CMessage orginalMessage;
		if ( !common::CommunicationProtocol::unwindMessage( _messageResult.m_message, orginalMessage, GetTime(), context< CConnectNodeAction >().getPublicKey() ) )
			assert( !"service it somehow" );

		if ( orginalMessage.m_header.m_payloadKind == common::CPayloadKind::InfoReq )
		{
			common::CInfoRequestData infoRequest;

			common::convertPayload( orginalMessage, infoRequest );

			assert( infoRequest.m_kind == common::CInfoKind::RoleInfoAsk );

			common::CSendMessageRequest * request =
					new common::CSendMessageRequest(
						common::CPayloadKind::RoleInfo
						, context< CConnectNodeAction >().getActionKey()
						, _messageResult.m_message.m_header.m_id
						, new CSpecificMediumFilter( context< CConnectNodeAction >().getNodePtr() ) );

			request->addPayload(
						common::CNetworkRole( (int)common::CRole::Monitor ) );

			context< CConnectNodeAction >().addRequest( request );

		}
		else if ( orginalMessage.m_header.m_payloadKind == common::CPayloadKind::RoleInfo )
		{
			common::CNetworkRole networkRole;
			common::convertPayload( orginalMessage, networkRole );

			context< CConnectNodeAction >().forgetRequests();
			context< CConnectNodeAction >().addRequest(
						new common::CAckRequest(
							  context< CConnectNodeAction >().getActionKey()
							, _messageResult.m_message.m_header.m_id
							, new CSpecificMediumFilter( context< CConnectNodeAction >().getNodePtr() ) ) );

			context< CConnectNodeAction >().setRole( ( common::CRole::Enum )networkRole.m_role );

			switch ( networkRole.m_role )
			{
			case common::CRole::Tracker:
				LogPrintf("connect node action: %p connected to tracker \n", &context< CConnectNodeAction >() );
				return transit< CGetNetworkInfo >();
			case common::CRole::Seed:
				return transit< CMonitorConnectedToSeed >();
			case common::CRole::Monitor:
				LogPrintf("connect node action: %p connected to monitor \n", &context< CConnectNodeAction >() );
				return transit< CGetNetworkInfo >();
			default:
				break;
			}

		}

		return discard_event();
	}

	boost::statechart::result react( common::CAckEvent const & _ackEvent )
	{
		context< CConnectNodeAction >().forgetRequests();

		common::CSendMessageRequest * request =
				new common::CSendMessageRequest(
					common::CPayloadKind::InfoReq
					, context< CConnectNodeAction >().getActionKey()
					, new CSpecificMediumFilter( context< CConnectNodeAction >().getNodePtr() ) );

		request->addPayload( common::CInfoKind::RoleInfoAsk );

		context< CConnectNodeAction >().addRequest( request );

		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CMessageResult >,
	boost::statechart::custom_reaction< common::CAckEvent >
	> reactions;

	int m_role;
};

// it is  wrong but  for  now...
struct CMonitorCantReachNode : boost::statechart::state< CMonitorCantReachNode, CConnectNodeAction >
{
	CMonitorCantReachNode( my_context ctx ) : my_base( ctx )
	{
		common::CFailureEvent failureEvent;
		common::createPayload( context< CConnectNodeAction >().getServiceAddress(), failureEvent.m_problemData );

		LogPrintf("connect node action: %p can't reach node' \n", &context< CConnectNodeAction >() );
		context< CConnectNodeAction >().setResult(
					failureEvent );

		context< CConnectNodeAction >().setExit();
	}
};

struct CMonitorStop;

struct CMonitorConnectedToSeed : boost::statechart::state< CMonitorConnectedToSeed, CConnectNodeAction >
{
	CMonitorConnectedToSeed( my_context ctx ) : my_base( ctx )
	{
		LogPrintf("connect node action: %p connected to seed \n", &context< CConnectNodeAction >() );
		context< CConnectNodeAction >().addRequest(
					new common::CTimeEventRequest(
						LoopTime
						, new CMediumClassFilter( common::CMediumKinds::Time ) ) );
	}

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		context< CConnectNodeAction >().forgetRequests();
		context< CConnectNodeAction >().setExit();
		return discard_event();
	}

	boost::statechart::result react( common::CMessageResult const & _messageResult )
	{
		common::CMessage orginalMessage;
		if ( !common::CommunicationProtocol::unwindMessage( _messageResult.m_message, orginalMessage, GetTime(), context< CConnectNodeAction >().getPublicKey() ) )
			assert( !"service it somehow" );

		if ( orginalMessage.m_header.m_payloadKind == common::CPayloadKind::InfoReq )
		{
			common::CInfoRequestData infoRequest;

			common::convertPayload( orginalMessage, infoRequest );

			assert( infoRequest.m_kind == common::CInfoKind::NetworkInfoAsk );
			context< CConnectNodeAction >().forgetRequests();

			common::CKnownNetworkInfo knownNetworkInfo;
			knownNetworkInfo.m_trackersInfo = CReputationTracker::getInstance()->getNodesInfo( common::CRole::Tracker );
			knownNetworkInfo.m_monitorsInfo = CReputationTracker::getInstance()->getNodesInfo( common::CRole::Monitor );

			common::CSendMessageRequest * request =
					new common::CSendMessageRequest(
						common::CPayloadKind::NetworkInfo
						, context< CConnectNodeAction >().getActionKey()
						, _messageResult.m_message.m_header.m_id
						, new CSpecificMediumFilter( context< CConnectNodeAction >().getNodePtr() ) );

			request->addPayload( knownNetworkInfo );

			context< CConnectNodeAction >().addRequest( request );
		}

		return discard_event();
	}

	boost::statechart::result react( common::CAckEvent const & _promptAck )
	{
		context< CConnectNodeAction >().forgetRequests();
		context< CConnectNodeAction >().setExit();
		return transit< CMonitorStop >();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CMessageResult >,
	boost::statechart::custom_reaction< common::CTimeEvent >,
	boost::statechart::custom_reaction< common::CAckEvent >
	> reactions;
};

struct CMonitorStop : boost::statechart::state< CMonitorStop, CConnectNodeAction >
{
	CMonitorStop( my_context ctx ) : my_base( ctx )
	{
		context< CConnectNodeAction >().setExit();
		context< CConnectNodeAction >().forgetRequests();
	}
};

struct CGetNetworkInfo : boost::statechart::state< CGetNetworkInfo, CConnectNodeAction >
{
	CGetNetworkInfo( my_context ctx )
		: my_base( ctx )
		, m_infoSend(false)
		, m_infoReceive(false)
	{
		CReputationTracker::getInstance()->setKeyToNode(
					context< CConnectNodeAction >().getPublicKey()
					, context< CConnectNodeAction >().getNodePtr() );

		CReputationTracker::getInstance()->setNodeInfo(
					common::CValidNodeInfo(
						context< CConnectNodeAction >().getPublicKey()
						, context< CConnectNodeAction >().getServiceAddress() )
					, context< CConnectNodeAction >().getRole());

		context< CConnectNodeAction >().forgetRequests();

		common::CSendMessageRequest * request =
				new common::CSendMessageRequest(
					common::CPayloadKind::InfoReq
					, context< CConnectNodeAction >().getActionKey()
					, new CSpecificMediumFilter( context< CConnectNodeAction >().getNodePtr() ) );

		request->addPayload( common::CInfoKind::NetworkInfoAsk );

		context< CConnectNodeAction >().addRequest( request );

		context< CConnectNodeAction >().addRequest(
					new common::CTimeEventRequest(
						  LoopTime
						, new CMediumClassFilter( common::CMediumKinds::Time ) ) );
	}

	boost::statechart::result react( common::CMessageResult const & _messageResult )
	{
		common::CMessage orginalMessage;
		if ( !common::CommunicationProtocol::unwindMessage( _messageResult.m_message, orginalMessage, GetTime(), context< CConnectNodeAction >().getPublicKey() ) )
			assert( !"service it somehow" );

		if ( orginalMessage.m_header.m_payloadKind == common::CPayloadKind::InfoReq )
		{
			common::CInfoRequestData infoRequest;

			common::convertPayload( orginalMessage, infoRequest );

			assert( infoRequest.m_kind == common::CInfoKind::NetworkInfoAsk );

			common::CKnownNetworkInfo knownNetworkInfo;
			knownNetworkInfo.m_trackersInfo = CReputationTracker::getInstance()->getNodesInfo( common::CRole::Tracker );
			knownNetworkInfo.m_monitorsInfo = CReputationTracker::getInstance()->getNodesInfo( common::CRole::Monitor );

			common::CSendMessageRequest * request =
					new common::CSendMessageRequest(
						common::CPayloadKind::NetworkInfo
						, context< CConnectNodeAction >().getActionKey()
						, _messageResult.m_message.m_header.m_id
						, new CSpecificMediumFilter( context< CConnectNodeAction >().getNodePtr() ) );

			request->addPayload( knownNetworkInfo );

			context< CConnectNodeAction >().addRequest( request );

			m_infoSend = true;
		}
		else if ( orginalMessage.m_header.m_payloadKind == common::CPayloadKind::NetworkInfo )
		{
			common::CKnownNetworkInfo knownNetworkInfo;

			common::convertPayload( orginalMessage, knownNetworkInfo );

			context< CConnectNodeAction >().addRequest(
						new common::CAckRequest(
							  context< CConnectNodeAction >().getActionKey()
							, _messageResult.m_message.m_header.m_id
							, new CSpecificMediumFilter( context< CConnectNodeAction >().getNodePtr() ) ) );

			common::CNetworkInfoResult networkRoleInfo(
						  common::CValidNodeInfo( context< CConnectNodeAction >().getPublicKey(), context< CConnectNodeAction >().getServiceAddress() )
						, context< CConnectNodeAction >().getRole()
						, knownNetworkInfo.m_trackersInfo
						, knownNetworkInfo.m_monitorsInfo );

			m_infoReceive = true;

			context< CConnectNodeAction >().setResult( networkRoleInfo );
		}

		if ( m_infoReceive && m_infoSend )
		{
			context< CConnectNodeAction >().addRequest(
						new common::CTimeEventRequest(
							  1000
							, new CMediumClassFilter( common::CMediumKinds::Time ) ) );
		}

		return discard_event();
	}

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		context< CConnectNodeAction >().setExit();
		return discard_event();
	}

	boost::statechart::result react( common::CAckEvent const & _ackEvent )
	{
		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CTimeEvent >,
	boost::statechart::custom_reaction< common::CAckEvent >,
	boost::statechart::custom_reaction< common::CMessageResult >
	> reactions;

	bool m_infoSend;
	bool m_infoReceive;
};


CConnectNodeAction::CConnectNodeAction( uint256 const & _actionKey, uintptr_t _nodePtr )
	: common::CScheduleAbleAction( _actionKey )
	, m_passive( true )
	, m_nodePtr( _nodePtr )
{
	initiate();
	process_event( common::CSwitchToConnectedEvent() );
}

CConnectNodeAction::CConnectNodeAction( CAddress const & _addrConnect )
	: m_passive( false )
	, m_addrConnect( _addrConnect )
{
	for ( unsigned int i = 0; i < ms_randomPayloadLenght; i++ )
	{
		m_payload.push_back( insecure_rand() % 256 );
	}

	// !! use  default  no  matter  what  is  there
	// it  warks   but  is  it acctually correct??
	m_addrConnect.SetPort( common::dimsParams().GetDefaultPort() );

	initiate();
	process_event( common::CSwitchToConnectingEvent() );
}

void
CConnectNodeAction::accept( common::CSetResponseVisitor & _visitor )
{
	_visitor.visit( *this );
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

uintptr_t
CConnectNodeAction::getNodePtr() const
{
	return m_nodePtr;
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
CConnectNodeAction::setServiceAddress( CAddress const & _addrConnect )
{
	m_addrConnect = _addrConnect;
}

void
CConnectNodeAction::setNodePtr( uintptr_t _nodePtr )
{
	m_nodePtr = _nodePtr;
}

}
