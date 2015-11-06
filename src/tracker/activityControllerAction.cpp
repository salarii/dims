// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "core.h"

#include <boost/statechart/state.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/custom_reaction.hpp>

#include "common/setResponseVisitor.h"
#include "common/requests.h"
#include "common/events.h"

#include "tracker/filters.h"
#include "tracker/activityControllerAction.h"
#include "tracker/connectNodeAction.h"

namespace tracker
{

//milisec
unsigned int const WaitTime = 20000;

struct CInitiateActivationEvent : boost::statechart::event< CInitiateActivationEvent >{};

struct CRecognizeNodeStateEvent : boost::statechart::event< CRecognizeNodeStateEvent >{};

struct CInitiateActivation;
struct CRecognizeNodeState;

namespace
{
CPubKey Node;
CActivitySatatus::Enum Status;

}

struct CActivityInitial : boost::statechart::simple_state< CActivityInitial, CActivityControllerAction >
{
	typedef boost::mpl::list<
	boost::statechart::transition< CInitiateActivationEvent, CInitiateActivation >,
	boost::statechart::transition< CRecognizeNodeStateEvent, CRecognizeNodeState >
	> reactions;
};

struct CInitiateActivation : boost::statechart::state< CInitiateActivation, CActivityControllerAction >
{
	CInitiateActivation( my_context ctx )
		: my_base( ctx )
	{
		LogPrintf("activity controller action: %p initiate activation \n", &context< CActivityControllerAction >() );

		// for  now  don't care  about result
		context< CActivityControllerAction >().addRequest(
				new common::CSendMessageRequest(
					common::CPayloadKind::ActivationStatus
					, common::CActivationStatus( Node.GetID(),(int)Status )
					, context< CActivityControllerAction >().getActionKey()
					, new CMediumClassFilter( common::CMediumKinds::DimsNodes ) ) );

		context< CActivityControllerAction >().addRequest(
					new common::CTimeEventRequest(
						WaitTime
						, new CMediumClassFilter( common::CMediumKinds::Time ) ) );
	}

	boost::statechart::result react( common::CAckEvent const & _ackEvent )
	{
		context< CActivityControllerAction >().setExit();// simplified
		return discard_event();
	}

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		assert(!"may consider as  problem");
		context< CActivityControllerAction >().setExit();
		return discard_event();
	}

	boost::statechart::result react( common::CNoMedium const & _noMedium )
	{
		context< CActivityControllerAction >().setExit();
		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CAckEvent >,
	boost::statechart::custom_reaction< common::CTimeEvent >
	> reactions;
};

struct CRecognizeNodeState : boost::statechart::state< CRecognizeNodeState, CActivityControllerAction >
{
	CRecognizeNodeState( my_context ctx )
		: my_base( ctx )
	{
		m_alreadyInformed = false;
	}

	boost::statechart::result react( common::CMessageResult const & _messageResult )
	{
		std::set< CPubKey > alreadyInformed;

		m_lastKey = _messageResult.m_pubKey;

		common::CMessage orginalMessage;
		if ( !common::CommunicationProtocol::unwindMessageAndParticipants( _messageResult.m_message, orginalMessage, GetTime(), _messageResult.m_pubKey, alreadyInformed ) )
			assert( !"service it somehow" );

		BOOST_FOREACH( CPubKey const & pubKey, alreadyInformed )
		{
			m_informingNodes.insert( pubKey.GetID() );
		}

		if ( orginalMessage.m_header.m_payloadKind == common::CPayloadKind::ActivationStatus && !m_alreadyInformed )
		{
			m_alreadyInformed = true;

			common::CActivationStatus activationStatus;

			common::convertPayload( orginalMessage, activationStatus );

			context< CActivityControllerAction >().addRequest(
						new common::CSendMessageRequest(
							common::CPayloadKind::Ack
							, common::CAck()
							, context< CActivityControllerAction >().getActionKey()
							, _messageResult.m_message.m_header.m_id
							, new CByKeyMediumFilter( _messageResult.m_pubKey ) ) );

			if ( activationStatus.m_status == CActivitySatatus::Active )
			{
				CTrackerNodesManager::getInstance()->setActiveNode( activationStatus.m_keyId );
			}
			else if ( activationStatus.m_status == CActivitySatatus::Inactive )
			{
				m_keyId = activationStatus.m_keyId;

				common::CValidNodeInfo validNodeInfo;
				if ( CTrackerNodesManager::getInstance()->getNodeInfo( activationStatus.m_keyId, validNodeInfo ) )
				{
					context< CActivityControllerAction >().addRequest(
								new common::CScheduleActionRequest(
									new CConnectNodeAction( validNodeInfo.m_address )
									, new CMediumClassFilter( common::CMediumKinds::Schedule) ) );
				}
			}
		}

		return discard_event();
	}

	boost::statechart::result react( common::CAckEvent const & _ackEvent )
	{
		context< CActivityControllerAction >().setExit();// simplified
		return discard_event();
	}

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		assert(!"may consider as  problem");
		context< CActivityControllerAction >().setExit();
		return discard_event();
	}

	boost::statechart::result react( common::CNoMedium const & _noMedium )
	{
		context< CActivityControllerAction >().setExit();
		return discard_event();
	}

	boost::statechart::result react( common::CNetworkInfoResult const & _networkInfoEvent )
	{

		if ( !_networkInfoEvent.m_valid )
		{
			CTrackerNodesManager::getInstance()->removeActiveNode( m_keyId );

			//m_alreadyInformed
			context< CActivityControllerAction >().addRequest(
						new common::CSendMessageRequest(
							m_message
							, m_lastKey
							, context< CActivityControllerAction >().getActionKey()
							, new CNodeExceptionFilter( common::CMediumKinds::DimsNodes, m_informingNodes ) ) );
		}

		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CAckEvent >,
	boost::statechart::custom_reaction< common::CTimeEvent >,
	boost::statechart::custom_reaction< common::CMessageResult >,
	boost::statechart::custom_reaction< common::CNetworkInfoResult >
	> reactions;

	std::set< uint160 > m_informingNodes;
	common::CMessage m_message;
	CPubKey m_lastKey;
	uint160 m_keyId;
	bool m_alreadyInformed;
};

CActivityControllerAction::CActivityControllerAction( CPubKey const & _node, CActivitySatatus::Enum _status )
{
	Node = _node;
	Status = _status;
	initiate();
	process_event( CInitiateActivationEvent() );
}

CActivityControllerAction::CActivityControllerAction( uint256 const & _actionKey )
	: common::CAction( _actionKey )
{
	initiate();
	process_event( CRecognizeNodeStateEvent() );
}

void
CActivityControllerAction::accept( common::CSetResponseVisitor & _visitor )
{
	_visitor.visit( *this );
}

}
