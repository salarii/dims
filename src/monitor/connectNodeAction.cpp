// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "connectNodeAction.h"
#include "common/setResponseVisitor.h"
#include "common/commonEvents.h"
#include "common/authenticationProvider.h"
#include "common/commonRequests.h"

#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/state.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/custom_reaction.hpp>

#include "monitor/filters.h"
#include "monitor/monitorRequests.h"
#include "monitor/reputationTracer.h"
#include "monitor/monitorController.h"

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

common::CRequest< common::CMonitorTypes > *
createIdentifyResponse( 	std::vector<unsigned char> const &_payload, uint256 const & _actionKey,common::CMonitorMediumFilter* _medium )
{
	uint256 hash = Hash( &_payload.front(), &_payload.back() );

	std::vector< unsigned char > signedHash;
	common::CAuthenticationProvider::getInstance()->sign( hash, signedHash );

	return new common::CSendIdentifyDataRequest< common::CMonitorTypes >( signedHash, common::CAuthenticationProvider::getInstance()->getMyKey(), _payload, _actionKey, _medium );
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

		context< CConnectNodeAction >().dropRequests();
		context< CConnectNodeAction >().addRequests(
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
		CReputationTracker::getInstance()->addNode( new common::CNodeMedium< common::CMonitorBaseMedium >( connectedEvent->m_node ) );
		context< CConnectNodeAction >().dropRequests();
		context< CConnectNodeAction >().addRequests(
					createIdentifyResponse(
						context< CConnectNodeAction >().getPayload(),
						context< CConnectNodeAction >().getActionKey(),
						new CSpecificMediumFilter( context< CConnectNodeAction >().getNodePtr() )
						)
					);
	}

	typedef boost::mpl::list<
	boost::statechart::transition< common::CAckEvent, CMonitorPairIdentifiedConnecting >
	> reactions;
};


struct CMonitorPairIdentifiedConnecting : boost::statechart::state< CMonitorPairIdentifiedConnecting, CConnectNodeAction >
{
	CMonitorPairIdentifiedConnecting( my_context ctx ) : my_base( ctx )
	{
		LogPrintf("connect node action: %p pair identified connecting \n", &context< CConnectNodeAction >() );
	}

	boost::statechart::result react( common::CIdentificationResult const & _identificationResult )
	{
		uint256 hash = Hash( &_identificationResult.m_payload.front(), &_identificationResult.m_payload.back() );

		if ( _identificationResult.m_key.Verify( hash, _identificationResult.m_signed ) )
		{
			context< CConnectNodeAction >().setPublicKey( _identificationResult.m_key );

			context< CConnectNodeAction >().dropRequests();

			context< CConnectNodeAction >().addRequests(
						new common::CAckRequest< common::CMonitorTypes >(
							  context< CConnectNodeAction >().getActionKey()
							, _identificationResult.m_id
							, new CSpecificMediumFilter( context< CConnectNodeAction >().getNodePtr() ) ) );

		}
		else
		{
			// something  is  wrong  with  pair react  somehow for  now put 0
			context< CConnectNodeAction >().dropRequests();
		}
		return transit< CMonitorDetermineRoleConnecting >();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CIdentificationResult >
	> reactions;
};

struct CMonitorDetermineRoleConnecting : boost::statechart::state< CMonitorDetermineRoleConnecting, CConnectNodeAction >
{
	CMonitorDetermineRoleConnecting( my_context ctx ) : my_base( ctx )
	{
		LogPrintf("connect node action: %p determine role connecting \n", &context< CConnectNodeAction >() );

		context< CConnectNodeAction >().addRequests(
					new common::CInfoAskRequest< common::CMonitorTypes >(
						  common::CInfoKind::RoleInfoAsk
						, context< CConnectNodeAction >().getActionKey()
						, new CSpecificMediumFilter( context< CConnectNodeAction >().getNodePtr() ) ) );

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

			context< CConnectNodeAction >().addRequests(
						new common::CNetworkRoleRequest< common::CMonitorTypes >(
							common::CRole::Monitor
							, context< CConnectNodeAction >().getActionKey()
							, _messageResult.m_message.m_header.m_id
							, new CSpecificMediumFilter( context< CConnectNodeAction >().getNodePtr() ) ) );
		}
		else if ( orginalMessage.m_header.m_payloadKind == common::CPayloadKind::RoleInfo )
		{
			common::CNetworkRole networkRole;
			common::convertPayload( orginalMessage, networkRole );

			context< CConnectNodeAction >().dropRequests();
			context< CConnectNodeAction >().addRequests(
						new common::CAckRequest< common::CMonitorTypes >(
							context< CConnectNodeAction >().getActionKey()
							, _messageResult.m_message.m_header.m_id
							, new CSpecificMediumFilter( context< CConnectNodeAction >().getNodePtr() ) ) );

			m_role = ( common::CRole::Enum )networkRole.m_role;
		}
		return discard_event();
	}

	boost::statechart::result react( common::CAckEvent const & _ackEvent )
	{

		switch ( m_role )
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

	common::CRole::Enum m_role;
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

			context< CConnectNodeAction >().dropRequests();

			context< CConnectNodeAction >().addRequests(
						new common::CAckRequest< common::CMonitorTypes >(
							  context< CConnectNodeAction >().getActionKey()
							, _identificationResult.m_id
							, new CSpecificMediumFilter( context< CConnectNodeAction >().getNodePtr() ) ) );

			context< CConnectNodeAction >().addRequests(
						createIdentifyResponse(
							_identificationResult.m_payload,
							context< CConnectNodeAction >().getActionKey(),
							new CSpecificMediumFilter( context< CConnectNodeAction >().getNodePtr() )
							)
						);
		}
		else
		{
			// something  is  wrong  with  pair react  somehow for  now put 0
			context< CConnectNodeAction >().dropRequests();
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

			context< CConnectNodeAction >().addRequests(
						new common::CNetworkRoleRequest< common::CMonitorTypes >(
							  common::CRole::Monitor
							, context< CConnectNodeAction >().getActionKey()
							, _messageResult.m_message.m_header.m_id
							, new CSpecificMediumFilter( context< CConnectNodeAction >().getNodePtr() ) ) );
		}
		else if ( orginalMessage.m_header.m_payloadKind == common::CPayloadKind::RoleInfo )
		{
			common::CNetworkRole networkRole;
			common::convertPayload( orginalMessage, networkRole );

			context< CConnectNodeAction >().dropRequests();
			context< CConnectNodeAction >().addRequests(
						new common::CAckRequest< common::CMonitorTypes >(
							  context< CConnectNodeAction >().getActionKey()
							, _messageResult.m_message.m_header.m_id
							, new CSpecificMediumFilter( context< CConnectNodeAction >().getNodePtr() ) ) );

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
		context< CConnectNodeAction >().dropRequests();

		context< CConnectNodeAction >().addRequests(
					new common::CInfoAskRequest< common::CMonitorTypes >(
						common::CInfoKind::RoleInfoAsk
						, context< CConnectNodeAction >().getActionKey()
						, new CSpecificMediumFilter( context< CConnectNodeAction >().getNodePtr() ) ) );

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
		LogPrintf("connect node action: %p can't reach node' \n", &context< CConnectNodeAction >() );
		context< CConnectNodeAction >().dropRequests();
	}
};

struct CMonitorStop;

struct CMonitorConnectedToSeed : boost::statechart::state< CMonitorConnectedToSeed, CConnectNodeAction >
{
	CMonitorConnectedToSeed( my_context ctx ) : my_base( ctx )
	{
		LogPrintf("connect node action: %p connected to seed \n", &context< CConnectNodeAction >() );
		context< CConnectNodeAction >().addRequests( new common::CTimeEventRequest< common::CMonitorTypes >( LoopTime, new CMediumClassFilter( common::CMediumKinds::Time ) ) );
	}

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		context< CConnectNodeAction >().dropRequests();
	}

	boost::statechart::result react( common::CMessageResult const & _result )
	{
		common::CMessage orginalMessage;
		if ( !common::CommunicationProtocol::unwindMessage( _result.m_message, orginalMessage, GetTime(), context< CConnectNodeAction >().getPublicKey() ) )
			assert( !"service it somehow" );

		common::CKnownNetworkInfo knownNetworkInfo;

		// save  this  stuff

		common::convertPayload( orginalMessage, knownNetworkInfo );// right  now it is not clear to me what to  do with  this

		context< CConnectNodeAction >().dropRequests();
		context< CConnectNodeAction >().addRequests( new common::CKnownNetworkInfoRequest< common::CMonitorTypes >( context< CConnectNodeAction >().getActionKey(), knownNetworkInfo, new CSpecificMediumFilter( context< CConnectNodeAction >().getNodePtr() ) ) );
		context< CConnectNodeAction >().addRequests( new common::CTimeEventRequest< common::CMonitorTypes >( LoopTime, new CMediumClassFilter( common::CMediumKinds::Time ) ) );
		return discard_event();
	}

	boost::statechart::result react( common::CAckEvent const & _promptAck )
	{
		context< CConnectNodeAction >().dropRequests();
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
		context< CConnectNodeAction >().dropRequests();
	}
};

struct CGetNetworkInfo : boost::statechart::state< CGetNetworkInfo, CConnectNodeAction >
{
	CGetNetworkInfo( my_context ctx ) : my_base( ctx )
	{
		CReputationTracker::getInstance()->setKeyToNode(
					context< CConnectNodeAction >().getPublicKey()
					, context< CConnectNodeAction >().getNodePtr() );

		context< CConnectNodeAction >().dropRequests();
		context< CConnectNodeAction >().addRequests(
					  new common::CInfoAskRequest< common::CMonitorTypes >(
						  common::CInfoKind::NetworkInfoAsk
						, context< CConnectNodeAction >().getActionKey()
						, new CSpecificMediumFilter( context< CConnectNodeAction >().getNodePtr() ) ) );

		context< CConnectNodeAction >().addRequests(
					new common::CTimeEventRequest< common::CMonitorTypes >(
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
			context< CConnectNodeAction >().dropRequests();

			common::CKnownNetworkInfo knownNetworkInfo;
			knownNetworkInfo.m_trackersInfo = CReputationTracker::getInstance()->getNodesInfo( common::CRole::Tracker );
			knownNetworkInfo.m_monitorsInfo = CReputationTracker::getInstance()->getNodesInfo( common::CRole::Monitor );

			context< CConnectNodeAction >().addRequests(
						new common::CKnownNetworkInfoRequest< common::CMonitorTypes >(
							  context< CConnectNodeAction >().getActionKey()
							, knownNetworkInfo
							, new CSpecificMediumFilter( context< CConnectNodeAction >().getNodePtr() ) ) );
		}
		else if ( orginalMessage.m_header.m_payloadKind == common::CPayloadKind::NetworkInfo )
		{
			common::CKnownNetworkInfo knownNetworkInfo;

			common::convertPayload( orginalMessage, knownNetworkInfo );

			context< CConnectNodeAction >().dropRequests();

			context< CConnectNodeAction >().addRequests(
						new common::CAckRequest< common::CMonitorTypes >(
							  context< CConnectNodeAction >().getActionKey()
							, _messageResult.m_message.m_header.m_id
							, new CSpecificMediumFilter( context< CConnectNodeAction >().getNodePtr() ) ) );

			common::CNetworkInfoResult networkRoleInfo(
								  context< CConnectNodeAction >().getPublicKey()
								, knownNetworkInfo.m_monitorsInfo
								, knownNetworkInfo.m_trackersInfo );

			context< CConnectNodeAction >().setResult( networkRoleInfo );
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
	boost::statechart::custom_reaction< common::CAckEvent >
	> reactions;
};


CConnectNodeAction::CConnectNodeAction( uint256 const & _actionKey, uintptr_t _nodePtr )
	: common::CScheduleAbleAction< common::CMonitorTypes >( _actionKey )
	, CCommunicationAction( _actionKey )
	, m_passive( true )
	, m_nodePtr( _nodePtr )
{
	initiate();
	process_event( common::CSwitchToConnectedEvent() );
}

CConnectNodeAction::CConnectNodeAction( CAddress const & _addrConnect )
	: CCommunicationAction( getActionKey() )
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

void
CConnectNodeAction::accept( common::CSetResponseVisitor< common::CMonitorTypes > & _visitor )
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
CConnectNodeAction::setNodePtr( uintptr_t _nodePtr )
{
	m_nodePtr = _nodePtr;
}

}
