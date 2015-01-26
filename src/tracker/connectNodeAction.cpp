// Copyright (c) 2014-2015 Dims dev-team
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
#include "trackerController.h"
#include "trackerControllerEvents.h"
#include "trackerNodeMedium.h"

#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/state.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/custom_reaction.hpp>

namespace tracker
{

unsigned int const TrackerLoopTime = 20;
unsigned int const SeedLoopTime = 25;
unsigned int const MonitorLoopTime = 25;

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

struct CDetermineRoleConnecting : boost::statechart::state< CDetermineRoleConnecting, CConnectNodeAction >
{
	CDetermineRoleConnecting( my_context ctx ) : my_base( ctx )
	{
		context< CConnectNodeAction >().setRequest( new common::CNetworkRoleRequest<TrackerResponses>( context< CConnectNodeAction >().getActionKey(), common::CRole::Tracker, new CSpecificMediumFilter( context< CConnectNodeAction >().getMediumPtr() ) ) );
	}

	boost::statechart::result react( common::CRoleEvent const & _roleEvent )
	{
		m_role = _roleEvent.m_role;
		context< CConnectNodeAction >().setRequest( new common::CAckRequest< TrackerResponses >( context< CConnectNodeAction >().getActionKey(), new CSpecificMediumFilter( context< CConnectNodeAction >().getMediumPtr() ) ) );
	}

	boost::statechart::result react( common::CContinueEvent const & _continueEvent )
	{
		context< CConnectNodeAction >().setRequest( new common::CContinueReqest<TrackerResponses>( _continueEvent.m_keyId, new CSpecificMediumFilter( context< CConnectNodeAction >().getMediumPtr() ) ) );
		return discard_event();
	}

	boost::statechart::result react( common::CAckPromptResult const & _promptAck )
	{
		switch ( m_role )
		{
		case common::CRole::Tracker:
			CTrackerController::getInstance()->process_event( CConnectedToTrackerEvent() );
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
		context< CConnectNodeAction >().setRequest( new common::CContinueReqest<TrackerResponses>( context< CConnectNodeAction >().getActionKey(), new CSpecificMediumFilter( context< CConnectNodeAction >().getMediumPtr() ) ) );
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CRoleEvent >,
	boost::statechart::custom_reaction< common::CContinueEvent >,
	boost::statechart::custom_reaction< common::CAckPromptResult >,
	boost::statechart::custom_reaction< common::CAckEvent >
	> reactions;

	int m_role;
};

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
			context< CConnectNodeAction >().setRequest( new common::CAckRequest< TrackerResponses >( context< CConnectNodeAction >().getActionKey(), new CSpecificMediumFilter( context< CConnectNodeAction >().getMediumPtr() ) ) );
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
	boost::statechart::transition< common::CAckEvent, CDetermineRoleConnecting >
	> reactions;

};

struct CDetermineRoleConnected : boost::statechart::state< CDetermineRoleConnected, CConnectNodeAction >
{
	CDetermineRoleConnected( my_context ctx ) : my_base( ctx )
	{
		context< CConnectNodeAction >().setRequest( new common::CContinueReqest<TrackerResponses>( context< CConnectNodeAction >().getActionKey(), new CSpecificMediumFilter( context< CConnectNodeAction >().getMediumPtr() ) ) );
	}

	boost::statechart::result react( common::CRoleEvent const & _roleEvent )
	{
		m_role = _roleEvent.m_role;
		context< CConnectNodeAction >().setRequest( new common::CAckRequest< TrackerResponses >( context< CConnectNodeAction >().getActionKey(), new CSpecificMediumFilter( context< CConnectNodeAction >().getMediumPtr() ) ) );
		return discard_event();
	}

	boost::statechart::result react( const common::CContinueEvent & _continueEvent )
	{
		context< CConnectNodeAction >().setRequest( new common::CContinueReqest<TrackerResponses>( _continueEvent.m_keyId, new CSpecificMediumFilter( context< CConnectNodeAction >().getMediumPtr() ) ) );
		return discard_event();
	}

	boost::statechart::result react( common::CAckPromptResult const & _ackPrompt )
	{
		context< CConnectNodeAction >().setRequest( new common::CNetworkRoleRequest<TrackerResponses>( context< CConnectNodeAction >().getActionKey(), common::CRole::Tracker, new CSpecificMediumFilter( context< CConnectNodeAction >().getMediumPtr() ) ) );
	}

	boost::statechart::result react( common::CAckEvent const & _ackEvent )
	{
		switch ( m_role )
		{
		case common::CRole::Tracker:
			CTrackerController::getInstance()->process_event( CTrackerConnectingEvent() );
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

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CRoleEvent >,
	boost::statechart::custom_reaction< common::CContinueEvent >,
	boost::statechart::custom_reaction< common::CAckPromptResult >,
	boost::statechart::custom_reaction< common::CAckEvent >
	> reactions;

	int m_role;
};

struct CPairIdentifiedConnected : boost::statechart::state< CPairIdentifiedConnected, CConnectNodeAction >
{
	CPairIdentifiedConnected( my_context ctx ) : my_base( ctx )
	{
		context< CConnectNodeAction >().setRequest( new common::CContinueReqest<TrackerResponses>( context< CConnectNodeAction >().getActionKey(), new CSpecificMediumFilter( context< CConnectNodeAction >().getMediumPtr() ) ) );
	}

	boost::statechart::result react( common::CContinueEvent const & _continueEvent )
	{
		context< CConnectNodeAction >().setRequest( new common::CContinueReqest<TrackerResponses>( _continueEvent.m_keyId, new CSpecificMediumFilter( context< CConnectNodeAction >().getMediumPtr() ) ) );
		return discard_event();
	}

	boost::statechart::result react( common::CIntroduceEvent const & _introduceEvent )
	{
		uint256 hash = Hash( &_introduceEvent.m_payload.front(), &_introduceEvent.m_payload.back() );

		if ( _introduceEvent.m_key.Verify( hash, _introduceEvent.m_signed ) )
		{
			CTrackerNodesManager::getInstance()->setPublicKey( _introduceEvent.m_address, _introduceEvent.m_key );
			context< CConnectNodeAction >().setRequest( new common::CAckRequest< TrackerResponses >( context< CConnectNodeAction >().getActionKey(), new CSpecificMediumFilter( context< CConnectNodeAction >().getMediumPtr() ) ) );
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
	boost::statechart::transition< common::CAckPromptResult, CDetermineRoleConnected >
	> reactions;
};


struct CBothUnidentifiedConnecting : boost::statechart::state< CBothUnidentifiedConnecting, CConnectNodeAction >
{
	CBothUnidentifiedConnecting( my_context ctx ) : my_base( ctx )
	{

		common::CNodeConnectedEvent const* connectedEvent = dynamic_cast< common::CNodeConnectedEvent const* >( simple_state::triggering_event() );
		context< CConnectNodeAction >().setMediumPtr( convertToInt( connectedEvent->m_node ) );
		// looks funny that  I set it in this  state, but let  it  be
		CTrackerNodesManager::getInstance()->addNode( new CTrackerNodeMedium( connectedEvent->m_node ) );

		context< CConnectNodeAction >().setRequest( new common::CIdentifyRequest<TrackerResponses>( new CSpecificMediumFilter( convertToInt( connectedEvent->m_node ) ), context< CConnectNodeAction >().getPayload(), context< CConnectNodeAction >().getActionKey() ) );
	}

	boost::statechart::result react( const common::CContinueEvent & _continueEvent )
	{
		context< CConnectNodeAction >().setRequest( new common::CContinueReqest<TrackerResponses>( _continueEvent.m_keyId, new CSpecificMediumFilter( context< CConnectNodeAction >().getMediumPtr() ) ) );
		return discard_event();
	}

	boost::statechart::result react( common::CAckEvent const & _ackEvent )
	{
		context< CConnectNodeAction >().setRequest( new common::CContinueReqest<TrackerResponses>( context< CConnectNodeAction >().getActionKey(), new CSpecificMediumFilter( context< CConnectNodeAction >().getMediumPtr() ) ) );
		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::transition< common::CIntroduceEvent, CPairIdentifiedConnecting >,
	boost::statechart::custom_reaction< common::CContinueEvent >,
	boost::statechart::custom_reaction< common::CAckEvent >
	> reactions;
};

struct CBothUnidentifiedConnected : boost::statechart::state< CBothUnidentifiedConnected, CConnectNodeAction >
{
	CBothUnidentifiedConnected( my_context ctx ) : my_base( ctx )
	{
		context< CConnectNodeAction >().setRequest( new common::CAckRequest< TrackerResponses >( context< CConnectNodeAction >().getActionKey(), new CSpecificMediumFilter( context< CConnectNodeAction >().getMediumPtr() ) ) );
	}

	boost::statechart::result react( const common::CContinueEvent & _continueEvent )
	{
		context< CConnectNodeAction >().setRequest( new common::CContinueReqest<TrackerResponses>( _continueEvent.m_keyId, new CSpecificMediumFilter( context< CConnectNodeAction >().getMediumPtr() ) ) );
		return discard_event();
	}

	boost::statechart::result react( common::CAckPromptResult const & _ackPrompt )
	{
		createIdentifyResponse( context< CConnectNodeAction >() );
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CContinueEvent >,
	boost::statechart::custom_reaction< common::CAckPromptResult >,
	boost::statechart::transition< common::CAckEvent, CPairIdentifiedConnected >
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

	boost::statechart::result react( common::CAckPromptResult const & _promptAck )
	{
		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::transition< common::CNodeConnectedEvent, CBothUnidentifiedConnecting >,
	boost::statechart::transition< common::CCantReachNode, CCantReachNode >,
	boost::statechart::custom_reaction< common::CAckPromptResult >
	> reactions;

};

struct ConnectedToTracker : boost::statechart::state< ConnectedToTracker, CConnectNodeAction >
{
	ConnectedToTracker( my_context ctx ) : my_base( ctx )
	{
		context< CConnectNodeAction >().setRequest( new common::CContinueReqest<TrackerResponses>( context< CConnectNodeAction >().getActionKey(), new CSpecificMediumFilter( context< CConnectNodeAction >().getMediumPtr() ) ) );

		((common::CNodesManager< TrackerResponses >*)CTrackerNodesManager::getInstance())
				->setValidNode( context< CConnectNodeAction >().getMediumPtr() );

		common::CAuthenticationProvider::getInstance()->addPubKey( context< CConnectNodeAction >().getPublicKey() );

		m_enterStateTime = GetTime();
	}

	boost::statechart::result react( const common::CContinueEvent & _continueEvent )
	{
		int64_t time = GetTime();
		if ( time - m_enterStateTime < TrackerLoopTime )
		{
			context< CConnectNodeAction >().setRequest( new common::CContinueReqest<TrackerResponses>( _continueEvent.m_keyId, new CSpecificMediumFilter( context< CConnectNodeAction >().getMediumPtr() ) ) );
		}
		else
		{
			context< CConnectNodeAction >().setRequest( 0 );
		}
		return discard_event();
	}

	boost::statechart::result react( common::CAckPromptResult const & _promptAck )
	{
		return discard_event();
	}

	int64_t m_enterStateTime;

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CContinueEvent >,
	boost::statechart::custom_reaction< common::CAckPromptResult >
	> reactions;
};

struct CStop;

struct ConnectedToSeed : boost::statechart::state< ConnectedToSeed, CConnectNodeAction >
{
	ConnectedToSeed( my_context ctx ) : my_base( ctx )
	{
		m_enterStateTime = GetTime();
		context< CConnectNodeAction >().setRequest( new common::CContinueReqest<TrackerResponses>( context< CConnectNodeAction >().getActionKey(), new CSpecificMediumFilter( context< CConnectNodeAction >().getMediumPtr() ) ) );
	}

	boost::statechart::result react( const common::CContinueEvent & _continueEvent )
	{
		int64_t time = GetTime();
		if ( time - m_enterStateTime < SeedLoopTime )
		{
			context< CConnectNodeAction >().setRequest( new common::CContinueReqest<TrackerResponses>( _continueEvent.m_keyId, new CSpecificMediumFilter( context< CConnectNodeAction >().getMediumPtr() ) ) );
		}
		else
		{
			context< CConnectNodeAction >().setRequest( 0 );
		}
		return discard_event();
	}

	boost::statechart::result react( common::CNetworkInfoEvent const & _networkInfo )
	{
		std::vector< common::CValidNodeInfo > validNodesInfo;

		BOOST_FOREACH( common::CValidNodeInfo const & validNodeInfo, CTrackerNodesManager::getInstance()->getValidNodes() )
		{
			validNodesInfo.push_back( validNodeInfo );
		}
		context< CConnectNodeAction >().setRequest( new common::CKnownNetworkInfoRequest< TrackerResponses >( context< CConnectNodeAction >().getActionKey(), validNodesInfo, new CSpecificMediumFilter( context< CConnectNodeAction >().getMediumPtr() ) ) );
		return discard_event();
	}

	boost::statechart::result react( common::CAckEvent const & _promptAck )
	{
		return transit< CStop >();
	}

	int64_t m_enterStateTime;

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CContinueEvent >,
	boost::statechart::custom_reaction< common::CNetworkInfoEvent >,
	boost::statechart::custom_reaction< common::CAckEvent >
	> reactions;
};

struct ConnectedToMonitor : boost::statechart::state< ConnectedToMonitor, CConnectNodeAction >
{
	ConnectedToMonitor( my_context ctx ) : my_base( ctx )
	{
		m_enterStateTime = GetTime();
		context< CConnectNodeAction >().setRequest( new common::CContinueReqest<TrackerResponses>( context< CConnectNodeAction >().getActionKey(), new CSpecificMediumFilter( context< CConnectNodeAction >().getMediumPtr() ) ) );
	}


	boost::statechart::result react( common::CMessageResult const & _connectCondition )
	{
		common::CMessage orginalMessage;
		if ( !common::CommunicationProtocol::unwindMessage( _connectCondition.m_message, orginalMessage, GetTime(), context< CConnectNodeAction >().getPublicKey() ) )
			assert( !"service it somehow" );

		common::CConnectCondition connectCondition;

		common::convertPayload( orginalMessage, connectCondition );

		unsigned int result = 0;

		CMonitorData & acquireMonitorData = CTrackerController::getInstance()->acquireMonitorData();

		if ( !acquireMonitorData.m_isAdmitted )
		{
			if ( acquireMonitorData.m_allowAdmission )
			{
				if ( acquireMonitorData.m_accepableRatio >= ( double )connectCondition.m_price / ( double )connectCondition.m_period.GetLow64() )
				{
					result = 1;
				}
			}

		}
		context< CConnectNodeAction >().setRequest( new common::CResultRequest< TrackerResponses >( context< CConnectNodeAction >().getActionKey(), result, new CSpecificMediumFilter( context< CConnectNodeAction >().getMediumPtr() ) ) );

		if ( !result )
			 transit< CStop >();
	}

	boost::statechart::result react( const common::CContinueEvent & _continueEvent )
	{
		int64_t time = GetTime();
		if ( time - m_enterStateTime < MonitorLoopTime )
		{
			context< CConnectNodeAction >().setRequest( new common::CContinueReqest<TrackerResponses>( _continueEvent.m_keyId, new CSpecificMediumFilter( context< CConnectNodeAction >().getMediumPtr() ) ) );
		}
		else
		{
			context< CConnectNodeAction >().setRequest( 0 );
		}
		return discard_event();
	}

	boost::statechart::result react( common::CAckEvent const & _ackEvent )
	{
		return transit< CStop >();
	}

	int64_t m_enterStateTime;

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CContinueEvent >,
	boost::statechart::custom_reaction< common::CMessageResult >,
	boost::statechart::custom_reaction< common::CAckEvent >
	> reactions;
};

struct CStop : boost::statechart::state< CStop, CConnectNodeAction >
{
	CStop( my_context ctx ) : my_base( ctx )
	{
		context< CConnectNodeAction >().setRequest( 0 );
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

CConnectNodeAction::CConnectNodeAction( uint256 const & _actionKey, std::vector< unsigned char > const & _payload, uintptr_t _mediumPtr )
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
	common::CRequest< TrackerResponses >* request = m_request;
	m_request = 0;
	return request;
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

common::CRequest< TrackerResponses > const *
CConnectNodeAction::getRequest() const
{
	return m_request;
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
