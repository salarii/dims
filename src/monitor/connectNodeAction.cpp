// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "connectNodeAction.h"
#include "common/setResponseVisitor.h"
#include "common/commonEvents.h"
#include "common/authenticationProvider.h"
#include "common/mediumRequests.h"

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

unsigned int const TrackerLoopTime = 20;
unsigned int const SeedLoopTime = 25;

struct CMonitorUnconnected; struct CMonitorBothUnidentifiedConnected;


struct CMonitorConnectNodeActionUninitiated : boost::statechart::simple_state< CMonitorConnectNodeActionUninitiated, CConnectNodeAction >
{
	typedef boost::mpl::list<
	boost::statechart::transition< common::CSwitchToConnectingEvent, CMonitorUnconnected >,
	boost::statechart::transition< common::CSwitchToConnectedEvent, CMonitorBothUnidentifiedConnected >
	> reactions;

};

class CMonitorConnectedToSeed;

class CMonitorConnectedToMonitor;

class CMonitorConnectedToTracker;

template < class Parent >
void
createIdentifyResponse( Parent & parent )
{
	uint256 hash = Hash( &parent.getPayload().front(), &parent.getPayload().back() );

	std::vector< unsigned char > signedHash;
	common::CAuthenticationProvider::getInstance()->sign( hash, signedHash );

	parent.setRequest( new common::CIdentifyResponse<MonitorResponses>( new CSpecificMediumFilter( parent.getMediumPtr() ), signedHash, common::CAuthenticationProvider::getInstance()->getMyKey(), parent.getPayload(), parent.getActionKey() ) );
}

struct CMonitorDetermineRoleConnecting : boost::statechart::state< CMonitorDetermineRoleConnecting, CConnectNodeAction >
{
	CMonitorDetermineRoleConnecting( my_context ctx ) : my_base( ctx )
	{
		context< CConnectNodeAction >().setRequest( new common::CNetworkRoleRequest<MonitorResponses>( context< CConnectNodeAction >().getActionKey(), common::CRole::Monitor, new CSpecificMediumFilter( context< CConnectNodeAction >().getMediumPtr() ) ) );
	}

	boost::statechart::result react( common::CMessageResult const & _roleEvent )
	{
		common::CMessage orginalMessage;
		if ( !common::CommunicationProtocol::unwindMessage( _roleEvent.m_message, orginalMessage, GetTime(), context< CConnectNodeAction >().getPublicKey() ) )
			assert( !"service it somehow" );

		common::CNetworkRole networkRole;

		common::convertPayload( orginalMessage, networkRole );

		m_role = networkRole.m_role;
		context< CConnectNodeAction >().setRequest( new common::CAckRequest< MonitorResponses >( context< CConnectNodeAction >().getActionKey(), new CSpecificMediumFilter( context< CConnectNodeAction >().getMediumPtr() ) ) );

		return discard_event();
	}

	boost::statechart::result react( common::CContinueEvent const & _continueEvent )
	{
		context< CConnectNodeAction >().setRequest( new common::CContinueReqest<MonitorResponses>( _continueEvent.m_keyId, new CSpecificMediumFilter( context< CConnectNodeAction >().getMediumPtr() ) ) );
		return discard_event();
	}

	boost::statechart::result react( common::CAckPromptResult const & _promptAck )
	{
		switch ( m_role )
		{
		case common::CRole::Tracker:
			//CTrackerController::getInstance()->process_event( CConnectedToTrackerEvent() );
			return transit< CMonitorConnectedToTracker >();
		case common::CRole::Seed:
			return transit< CMonitorConnectedToSeed >();
		case common::CRole::Monitor:
			return transit< CMonitorConnectedToMonitor >();
		default:
			break;
		}
		return discard_event();
	}

	boost::statechart::result react( common::CAckEvent const & _ackEvent )
	{
		context< CConnectNodeAction >().setRequest( new common::CContinueReqest<MonitorResponses>( context< CConnectNodeAction >().getActionKey(), new CSpecificMediumFilter( context< CConnectNodeAction >().getMediumPtr() ) ) );
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CMessageResult >,
	boost::statechart::custom_reaction< common::CContinueEvent >,
	boost::statechart::custom_reaction< common::CAckPromptResult >,
	boost::statechart::custom_reaction< common::CAckEvent >
	> reactions;

	int m_role;
};

struct CMonitorPairIdentifiedConnecting : boost::statechart::state< CMonitorPairIdentifiedConnecting, CConnectNodeAction >
{
	CMonitorPairIdentifiedConnecting( my_context ctx ) : my_base( ctx )
	{
		common::CIntroduceEvent const* requestedEvent = dynamic_cast< common::CIntroduceEvent const* >( simple_state::triggering_event() );

		uint256 hash = Hash( &requestedEvent->m_payload.front(), &requestedEvent->m_payload.back() );

		if ( requestedEvent->m_key.Verify( hash, requestedEvent->m_signed ) )
		{
			context< CConnectNodeAction >().setPublicKey( requestedEvent->m_key );

			context< CConnectNodeAction >().setRequest( new common::CAckRequest< MonitorResponses >( context< CConnectNodeAction >().getActionKey(), new CSpecificMediumFilter( context< CConnectNodeAction >().getMediumPtr() ) ) );
		}
		else
		{
		// something  is  wrong  with  pair react  somehow for  now put 0
			context< CConnectNodeAction >().setRequest( 0 );
		}
	}

	boost::statechart::result react( common::CContinueEvent const & _continueEvent )
	{
		context< CConnectNodeAction >().setRequest( new common::CContinueReqest<MonitorResponses>( _continueEvent.m_keyId, new CSpecificMediumFilter( context< CConnectNodeAction >().getMediumPtr() ) ) );
		return discard_event();
	}

	boost::statechart::result react( common::CAckPromptResult const & _promptAck )
	{
		createIdentifyResponse( context< CConnectNodeAction >() );
		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CContinueEvent >,
	boost::statechart::custom_reaction< common::CAckPromptResult >,
	boost::statechart::transition< common::CAckEvent, CMonitorDetermineRoleConnecting >
	> reactions;

};

struct CMonitorDetermineRoleConnected : boost::statechart::state< CMonitorDetermineRoleConnected, CConnectNodeAction >
{
	CMonitorDetermineRoleConnected( my_context ctx ) : my_base( ctx )
	{
		context< CConnectNodeAction >().setRequest( new common::CContinueReqest<MonitorResponses>( context< CConnectNodeAction >().getActionKey(), new CSpecificMediumFilter( context< CConnectNodeAction >().getMediumPtr() ) ) );
	}

	boost::statechart::result react( common::CMessageResult const & _roleEvent )
	{
		common::CMessage orginalMessage;
		if ( !common::CommunicationProtocol::unwindMessage( _roleEvent.m_message, orginalMessage, GetTime(), context< CConnectNodeAction >().getPublicKey() ) )
			assert( !"service it somehow" );

		common::CNetworkRole networkRole;

		common::convertPayload( orginalMessage, networkRole );

		m_role = networkRole.m_role;
		context< CConnectNodeAction >().setRequest( new common::CAckRequest< MonitorResponses >( context< CConnectNodeAction >().getActionKey(), new CSpecificMediumFilter( context< CConnectNodeAction >().getMediumPtr() ) ) );

		return discard_event();
	}

	boost::statechart::result react( const common::CContinueEvent & _continueEvent )
	{
		context< CConnectNodeAction >().setRequest( new common::CContinueReqest<MonitorResponses>( _continueEvent.m_keyId, new CSpecificMediumFilter( context< CConnectNodeAction >().getMediumPtr() ) ) );
		return discard_event();
	}

	boost::statechart::result react( common::CAckPromptResult const & _ackPrompt )
	{
		context< CConnectNodeAction >().setRequest( new common::CNetworkRoleRequest<MonitorResponses>( context< CConnectNodeAction >().getActionKey(), common::CRole::Monitor, new CSpecificMediumFilter( context< CConnectNodeAction >().getMediumPtr() ) ) );
	}

	boost::statechart::result react( common::CAckEvent const & _ackEvent )
	{
		switch ( m_role )
		{
		case common::CRole::Tracker:
		//	CTrackerController::getInstance()->process_event( CTrackerConnectingEvent() );
			return transit< CMonitorConnectedToTracker >();
		case common::CRole::Seed:
			return transit< CMonitorConnectedToSeed >();
		case common::CRole::Monitor:
			return transit< CMonitorConnectedToMonitor >();
		default:
			break;
		}
		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CMessageResult >,
	boost::statechart::custom_reaction< common::CContinueEvent >,
	boost::statechart::custom_reaction< common::CAckPromptResult >,
	boost::statechart::custom_reaction< common::CAckEvent >
	> reactions;

	int m_role;
};

struct CMonitorPairIdentifiedConnected : boost::statechart::state< CMonitorPairIdentifiedConnected, CConnectNodeAction >
{
	CMonitorPairIdentifiedConnected( my_context ctx ) : my_base( ctx )
	{
		context< CConnectNodeAction >().setRequest( new common::CContinueReqest<MonitorResponses>( context< CConnectNodeAction >().getActionKey(), new CSpecificMediumFilter( context< CConnectNodeAction >().getMediumPtr() ) ) );
	}

	boost::statechart::result react( common::CContinueEvent const & _continueEvent )
	{
		context< CConnectNodeAction >().setRequest( new common::CContinueReqest<MonitorResponses>( _continueEvent.m_keyId, new CSpecificMediumFilter( context< CConnectNodeAction >().getMediumPtr() ) ) );
		return discard_event();
	}

	boost::statechart::result react( common::CIntroduceEvent const & _introduceEvent )
	{
		uint256 hash = Hash( &_introduceEvent.m_payload.front(), &_introduceEvent.m_payload.back() );

		if ( _introduceEvent.m_key.Verify( hash, _introduceEvent.m_signed ) )
		{
			context< CConnectNodeAction >().setPublicKey(  _introduceEvent.m_key );
			context< CConnectNodeAction >().setRequest( new common::CAckRequest< MonitorResponses >( context< CConnectNodeAction >().getActionKey(), new CSpecificMediumFilter( context< CConnectNodeAction >().getMediumPtr() ) ) );
		}
		else
		{
			context< CConnectNodeAction >().setRequest( 0 );
		}

		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CIntroduceEvent >,
	boost::statechart::custom_reaction< common::CContinueEvent >,
	boost::statechart::transition< common::CAckPromptResult, CMonitorDetermineRoleConnected >
	> reactions;
};


struct CMonitorBothUnidentifiedConnecting : boost::statechart::state< CMonitorBothUnidentifiedConnecting, CConnectNodeAction >
{
	CMonitorBothUnidentifiedConnecting( my_context ctx ) : my_base( ctx )
	{

		common::CNodeConnectedEvent const* connectedEvent = dynamic_cast< common::CNodeConnectedEvent const* >( simple_state::triggering_event() );
		context< CConnectNodeAction >().setMediumPtr( convertToInt( connectedEvent->m_node ) );
		// looks funny that  I set it in this  state, but let  it  be
		CReputationTracker::getInstance()->addNode( new common::CNodeMedium<MonitorResponses>( connectedEvent->m_node ) );
		context< CConnectNodeAction >().setRequest( new common::CIdentifyRequest<MonitorResponses>( new CSpecificMediumFilter( convertToInt( connectedEvent->m_node ) ), context< CConnectNodeAction >().getPayload(), context< CConnectNodeAction >().getActionKey() ) );
	}

	boost::statechart::result react( const common::CContinueEvent & _continueEvent )
	{
		context< CConnectNodeAction >().setRequest( new common::CContinueReqest<MonitorResponses>( _continueEvent.m_keyId, new CSpecificMediumFilter( context< CConnectNodeAction >().getMediumPtr() ) ) );
		return discard_event();
	}

	boost::statechart::result react( common::CAckEvent const & _ackEvent )
	{
		context< CConnectNodeAction >().setRequest( new common::CContinueReqest<MonitorResponses>( context< CConnectNodeAction >().getActionKey(), new CSpecificMediumFilter( context< CConnectNodeAction >().getMediumPtr() ) ) );
		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::transition< common::CIntroduceEvent, CMonitorPairIdentifiedConnecting >,
	boost::statechart::custom_reaction< common::CContinueEvent >,
	boost::statechart::custom_reaction< common::CAckEvent >
	> reactions;
};

struct CMonitorBothUnidentifiedConnected : boost::statechart::state< CMonitorBothUnidentifiedConnected, CConnectNodeAction >
{
	CMonitorBothUnidentifiedConnected( my_context ctx ) : my_base( ctx )
	{
		context< CConnectNodeAction >().setRequest( new common::CAckRequest< MonitorResponses >( context< CConnectNodeAction >().getActionKey(), new CSpecificMediumFilter( context< CConnectNodeAction >().getMediumPtr() ) ) );
	}

	boost::statechart::result react( const common::CContinueEvent & _continueEvent )
	{
		context< CConnectNodeAction >().setRequest( new common::CContinueReqest<MonitorResponses>( _continueEvent.m_keyId, new CSpecificMediumFilter( context< CConnectNodeAction >().getMediumPtr() ) ) );
		return discard_event();
	}

	boost::statechart::result react( common::CAckPromptResult const & _ackPrompt )
	{
		createIdentifyResponse( context< CConnectNodeAction >() );
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CContinueEvent >,
	boost::statechart::custom_reaction< common::CAckPromptResult >,
	boost::statechart::transition< common::CAckEvent, CMonitorPairIdentifiedConnected >
	> reactions;
};

// it is  wrong but  for  now...
struct CMonitorCantReachNode : boost::statechart::state< CMonitorCantReachNode, CConnectNodeAction >
{
	CMonitorCantReachNode( my_context ctx ) : my_base( ctx )
	{
		context< CConnectNodeAction >().setRequest( 0 );
	}
};

struct CMonitorUnconnected : boost::statechart::state< CMonitorUnconnected, CConnectNodeAction >
{
	CMonitorUnconnected( my_context ctx ) : my_base( ctx )
	{
		context< CConnectNodeAction >().setRequest(
				  new CConnectToNodeRequest( "", context< CConnectNodeAction >().getServiceAddress() ) );
	}

	boost::statechart::result react( common::CAckPromptResult const & _promptAck )
	{
		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::transition< common::CNodeConnectedEvent, CMonitorBothUnidentifiedConnecting >,
	boost::statechart::transition< common::CCantReachNode, CMonitorCantReachNode >,
	boost::statechart::custom_reaction< common::CAckPromptResult >
	> reactions;

};

struct CMonitorConnectedToTracker : boost::statechart::state< CMonitorConnectedToTracker, CConnectNodeAction >
{
	CMonitorConnectedToTracker( my_context ctx ) : my_base( ctx )
	{
		CReputationTracker::getInstance()->setKeyToNode(
				  context< CConnectNodeAction >().getPublicKey()
				, context< CConnectNodeAction >().getMediumPtr() );

		context< CConnectNodeAction >().setRequest( new CConnectCondition( context< CConnectNodeAction >().getActionKey(), CMonitorController::getInstance()->getPrice(), CMonitorController::getInstance()->getPeriod(), new CSpecificMediumFilter( context< CConnectNodeAction >().getMediumPtr() ) ) );
	}

	boost::statechart::result react( const common::CContinueEvent & _continueEvent )
	{
		context< CConnectNodeAction >().setRequest( new common::CContinueReqest<MonitorResponses>( _continueEvent.m_keyId, new CSpecificMediumFilter( context< CConnectNodeAction >().getMediumPtr() ) ) );

		return discard_event();
	}

	boost::statechart::result react( common::CMessageResult const & _result )
	{
		common::CMessage orginalMessage;
		if ( !common::CommunicationProtocol::unwindMessage( _result.m_message, orginalMessage, GetTime(), context< CConnectNodeAction >().getPublicKey() ) )
			assert( !"service it somehow" );

		common::CResult result;

		common::convertPayload( orginalMessage, result );

		if ( result.m_result )
		{

			if ( CMonitorController::getInstance()->getPrice() )
			{
				// create action send ack
			}
			else
			{
						context< CConnectNodeAction >().setRequest( new common::CAckRequest< MonitorResponses >( context< CConnectNodeAction >().getActionKey(), new CSpecificMediumFilter( context< CConnectNodeAction >().getMediumPtr() ) ) );
				CReputationTracker::getInstance()->addTracker( CTrackerData( context< CConnectNodeAction >().getServiceAddress(), 0, context< CConnectNodeAction >().getPublicKey(), CMonitorController::getInstance()->getPeriod(), GetTime() ) );
			}
		}
		else
		{
			// ask about status  of  this  tracker
		}
//		context< CConnectNodeAction >().setRequest( new common::CAckRequest< MonitorResponses >( context< CConnectNodeAction >().getActionKey(), new CSpecificMediumFilter( context< CConnectNodeAction >().getMediumPtr() ) ) );

		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CMessageResult >,
	boost::statechart::custom_reaction< common::CContinueEvent >
	> reactions;
};

struct CMonitorStop;

struct CMonitorConnectedToSeed : boost::statechart::state< CMonitorConnectedToSeed, CConnectNodeAction >
{
	CMonitorConnectedToSeed( my_context ctx ) : my_base( ctx )
	{
		m_enterStateTime = GetTime();
		context< CConnectNodeAction >().setRequest( new common::CContinueReqest<MonitorResponses>( context< CConnectNodeAction >().getActionKey(), new CSpecificMediumFilter( context< CConnectNodeAction >().getMediumPtr() ) ) );
	}

	boost::statechart::result react( const common::CContinueEvent & _continueEvent )
	{
		int64_t time = GetTime();
		if ( time - m_enterStateTime < SeedLoopTime )
		{
			context< CConnectNodeAction >().setRequest( new common::CContinueReqest<MonitorResponses>( _continueEvent.m_keyId, new CSpecificMediumFilter( context< CConnectNodeAction >().getMediumPtr() ) ) );
		}
		else
		{
			context< CConnectNodeAction >().setRequest( 0 );
		}
		return discard_event();
	}

	boost::statechart::result react( common::CMessageResult const & _result )
	{
		common::CMessage orginalMessage;
		if ( !common::CommunicationProtocol::unwindMessage( _result.m_message, orginalMessage, GetTime(), context< CConnectNodeAction >().getPublicKey() ) )
			assert( !"service it somehow" );

		common::CKnownNetworkInfo knownNetworkInfo;

		// save  this  stuff

		common::convertPayload( orginalMessage, knownNetworkInfo );// right  now it is not clear to me what to  do with  this

		context< CConnectNodeAction >().setRequest( new common::CKnownNetworkInfoRequest< MonitorResponses >( context< CConnectNodeAction >().getActionKey(), knownNetworkInfo, new CSpecificMediumFilter( context< CConnectNodeAction >().getMediumPtr() ) ) );

		return discard_event();
	}

	boost::statechart::result react( common::CAckEvent const & _promptAck )
	{
		return transit< CMonitorStop >();
	}

	int64_t m_enterStateTime;

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CContinueEvent >,
		boost::statechart::custom_reaction< common::CMessageResult >,
	boost::statechart::custom_reaction< common::CAckEvent >
	> reactions;
};

struct CMonitorStop : boost::statechart::state< CMonitorStop, CConnectNodeAction >
{
	CMonitorStop( my_context ctx ) : my_base( ctx )
	{
	}

	boost::statechart::result react( const common::CContinueEvent & _continueEvent )
	{
		context< CConnectNodeAction >().setRequest( 0 );
		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CContinueEvent >
	> reactions;
};


struct CMonitorConnectedToMonitor : boost::statechart::state< CMonitorConnectedToMonitor, CConnectNodeAction >
{
	CMonitorConnectedToMonitor( my_context ctx ) : my_base( ctx )
	{
		CReputationTracker::getInstance()->setKeyToNode(
				  context< CConnectNodeAction >().getPublicKey()
				, context< CConnectNodeAction >().getMediumPtr() );
	}
};

CConnectNodeAction::CConnectNodeAction( CAddress const & _addrConnect, uint256 const & _actionKey, std::vector< unsigned char > const & _payload, uintptr_t _mediumPtr )
: CCommunicationAction( _actionKey )
, m_addrConnect( _addrConnect )
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

common::CRequest< MonitorResponses >*
CConnectNodeAction::getRequest() const
{
	return m_request;
}

void
CConnectNodeAction::accept( common::CSetResponseVisitor< MonitorResponses > & _visitor )
{
	_visitor.visit( *this );
}

void
CConnectNodeAction::setRequest( common::CRequest< MonitorResponses >* _request )
{
	m_request = _request;
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
CConnectNodeAction::setMediumPtr( uintptr_t _mediumPtr )
{
	m_mediumPtr = _mediumPtr;
}

}
