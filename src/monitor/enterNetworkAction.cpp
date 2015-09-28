
// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "core.h"

#include <boost/statechart/state.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/custom_reaction.hpp>

#include "common/authenticationProvider.h"
#include "common/setResponseVisitor.h"
#include "common/analyseTransaction.h"
#include "common/requests.h"
#include "common/events.h"

#include "monitor/passTransactionAction.h"
#include "monitor/enterNetworkAction.h"
#include "monitor/controller.h"
#include "monitor/admitTransactionsBundle.h"
#include "monitor/filters.h"
#include "monitor/reputationTracer.h"
#include "monitor/monitorRequests.h"
#include "monitor/rankingDatabase.h"
#include "monitor/synchronizationAction.h"
#include "monitor/chargeRegister.h"

namespace monitor
{
struct CSynchronization;
struct CFreeRegistration;
struct CPaidRegistration;
struct CNetworkAlive;
struct CFreeRegistrationEnter;
struct CAssistAdmission;
struct CAdmissionCondition;
struct CAskForAddmision;
//milisec
unsigned int const WaitTime = 10000;

namespace // parameters  to  pass  between states
{
CPubKey monitorKey;
unsigned int price;
}

struct CSwitchToAdmit : boost::statechart::event< CSwitchToAdmit >{};

struct CSwitchToEnter : boost::statechart::event< CSwitchToEnter >{};

struct CEnterNetworkInitial : boost::statechart::simple_state< CEnterNetworkInitial, CEnterNetworkAction >
{
	typedef boost::mpl::list<
	boost::statechart::transition< CSwitchToAdmit, CAssistAdmission >,
	boost::statechart::transition< CSwitchToEnter, CAdmissionCondition >
	> reactions;
};

struct CAssistAdmission : boost::statechart::state< CAssistAdmission, CEnterNetworkAction >
{
	CAssistAdmission( my_context ctx )
		: my_base( ctx )
	{
	}

	boost::statechart::result react( common::CMessageResult const & _messageResult )
	{
		common::CMessage orginalMessage;
		if ( !common::CommunicationProtocol::unwindMessage( _messageResult.m_message, orginalMessage, GetTime(), _messageResult.m_pubKey ) )
			assert( !"service it somehow" );

		if ( _messageResult.m_message.m_header.m_payloadKind == common::CPayloadKind::EnterNetworkAsk )
		{
			context< CEnterNetworkAction >().forgetRequests();

			context< CEnterNetworkAction >().addRequest(
						new common::CAckRequest(
							context< CEnterNetworkAction >().getActionKey()
							, _messageResult.m_message.m_header.m_id
							, new CSpecificMediumFilter( _messageResult.m_nodeIndicator ) ) );

			common::CSendMessageRequest * request =
					new common::CSendMessageRequest(
						common::CPayloadKind::Result
						, context< CEnterNetworkAction >().getActionKey()
						, _messageResult.m_message.m_header.m_id
						, new CSpecificMediumFilter( _messageResult.m_nodeIndicator ) );

			request->addPayload( common::CResult( 1 ) );

			context< CEnterNetworkAction >().addRequest( request );
		}
		return discard_event();
	}

	boost::statechart::result react( common::CAckEvent const & _ackEvent )
	{
		if ( CController::getInstance()->getEnterancePrice() )
		{
			if ( CReputationTracker::getInstance()->getTrackers().empty() );
				//return transit< CPaidRegistrationEmptyNetwork >();
		}
		else
			return transit< CFreeRegistration >();

		return discard_event();
	}

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		context< CEnterNetworkAction >().setExit();
		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CAckEvent >,
	boost::statechart::custom_reaction< common::CTimeEvent >,
	boost::statechart::custom_reaction< common::CMessageResult >
	> reactions;
};

struct CFreeRegistration : boost::statechart::state< CFreeRegistration, CEnterNetworkAction >
{
	CFreeRegistration( my_context ctx )
		: my_base( ctx )
	{
		context< CEnterNetworkAction >().forgetRequests();

		context< CEnterNetworkAction >().addRequest(
					new common::CTimeEventRequest(
						WaitTime
						, new CMediumClassFilter( common::CMediumKinds::Time ) ) );
	}

	boost::statechart::result react( common::CMessageResult const & _messageResult )
	{
		common::CMessage orginalMessage;
		if ( !common::CommunicationProtocol::unwindMessage( _messageResult.m_message, orginalMessage, GetTime(), _messageResult.m_pubKey ) )
			assert( !"service it somehow" );

// do  something
		return discard_event();
	}

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		context< CEnterNetworkAction >().forgetRequests();
		return discard_event();
	}

	boost::statechart::result react( common::CAckEvent const & _ackEvent )
	{
		context< CEnterNetworkAction >().forgetRequests();
		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CAckEvent >,
	boost::statechart::custom_reaction< common::CTimeEvent >,
	boost::statechart::custom_reaction< common::CMessageResult >
	> reactions;

};

struct CPaidRegistration : boost::statechart::state< CPaidRegistration, CEnterNetworkAction >
{
	CPaidRegistration( my_context ctx )
		: my_base( ctx )
		, m_checkPeriod( 30000 )
	{
		context< CEnterNetworkAction >().forgetRequests();

		CChargeRegister::getInstance()->setStoreTransactions( true );
	}

	boost::statechart::result react( common::CMessageResult const & _messageResult )
	{
		common::CMessage orginalMessage;
		if ( !common::CommunicationProtocol::unwindMessage( _messageResult.m_message, orginalMessage, GetTime(), _messageResult.m_pubKey ) )
			assert( !"service it somehow" );

		common::CAdmitProof admitMessage;

		common::convertPayload( orginalMessage, admitMessage );

		CChargeRegister::getInstance()->addTransactionToSearch( admitMessage.m_proofTransactionHash, _messageResult.m_pubKey.GetID() );

		m_proofHash = admitMessage.m_proofTransactionHash;

		m_nodePtr = _messageResult.m_nodeIndicator;

		common::CSendMessageRequest * request =
				new common::CSendMessageRequest(
					common::CPayloadKind::Ack
					, context< CEnterNetworkAction >().getActionKey()
					, _messageResult.m_message.m_header.m_id
					, new CSpecificMediumFilter( _messageResult.m_nodeIndicator ) );

		request->addPayload( common::CAck() );

		context< CEnterNetworkAction >().addRequest( request );

		context< CEnterNetworkAction >().forgetRequests();
		context< CEnterNetworkAction >().addRequest(
					new common::CTimeEventRequest(
						m_checkPeriod
						, new CMediumClassFilter( common::CMediumKinds::Time ) ) );

		m_messageId = _messageResult.m_message.m_header.m_id;

		m_pubKey = _messageResult.m_pubKey;

		return discard_event();
	}

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		CTransaction transaction;

		if ( CChargeRegister::getInstance()->isTransactionPresent( m_proofHash ) )
		{

			common::CSendMessageRequest * request =
					new common::CSendMessageRequest(
						common::CPayloadKind::Result
						, context< CEnterNetworkAction >().getActionKey()
						, m_messageId
						, new CSpecificMediumFilter( m_nodePtr ) );

			uint160 pubKeyId;

			if ( CReputationTracker::getInstance()->getNodeToKey( context< CEnterNetworkAction >().getNodePtr(), pubKeyId ) )
			{
				request->addPayload( common::CResult( 1 ) );

							common::CTrackerData trackerData(
								m_pubKey
								, 0
								, CController::getInstance()->getPeriod()
								, GetTime() );

				CRankingDatabase::getInstance()->writeTrackerData( trackerData );

				CReputationTracker::getInstance()->addTracker( trackerData );
			}
			else
			{
				request->addPayload( common::CResult( 0 ) );
			}

			context< CEnterNetworkAction >().addRequest( request );
		}
		else
		{
			context< CEnterNetworkAction >().forgetRequests();

			context< CEnterNetworkAction >().addRequest(
						new common::CTimeEventRequest(
							m_checkPeriod
							, new CMediumClassFilter( common::CMediumKinds::Time ) ) );

		}
		return discard_event();
	}

	~CPaidRegistration()
	{
		CChargeRegister::getInstance()->setStoreTransactions( false );
	}

	boost::statechart::result react( common::CAckEvent const & _ackEvent )
	{
		context< CEnterNetworkAction >().setExit();
		return discard_event();
	}


	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CAckEvent >,
	boost::statechart::custom_reaction< common::CTimeEvent >,
	boost::statechart::custom_reaction< common::CMessageResult >
	> reactions;

	uint256 m_proofHash;

	uint256 m_messageId;

	int64_t const m_checkPeriod;

	CPubKey m_pubKey;

	uintptr_t m_nodePtr;
};

struct CAdmissionCondition : boost::statechart::state< CAdmissionCondition, CEnterNetworkAction >
{
	CAdmissionCondition( my_context ctx )
		: my_base( ctx )
	{
		context< CEnterNetworkAction >().forgetRequests();
		context< CEnterNetworkAction >().addRequest(
		 new common::CTimeEventRequest(
						WaitTime
						, new CMediumClassFilter( common::CMediumKinds::Time ) ) );

		common::CSendMessageRequest * request =
				new common::CSendMessageRequest(
					common::CPayloadKind::InfoReq
					, context< CEnterNetworkAction >().getActionKey()
					, new CSpecificMediumFilter( context< CEnterNetworkAction >().getNodePtr() ) ); // bit  risky to ask  this way

		request->addPayload( common::CInfoKind::EnterConditionAsk, std::vector<unsigned char>() );

		context< CEnterNetworkAction >().addRequest( request );
	}

	boost::statechart::result react( common::CMessageResult const & _messageResult )
	{
		common::CMessage orginalMessage;
		if ( !common::CommunicationProtocol::unwindMessage( _messageResult.m_message, orginalMessage, GetTime(), _messageResult.m_pubKey ) )
			assert( !"service it somehow" );

		if ( _messageResult.m_message.m_header.m_payloadKind == common::CPayloadKind::EnterNetworkCondition )
		{	common::CEnteranceTerms enteranceTerms;

			common::convertPayload( orginalMessage, enteranceTerms );

			context< CEnterNetworkAction >().addRequest(
						new common::CAckRequest(
							context< CEnterNetworkAction >().getActionKey()
							, _messageResult.m_message.m_header.m_id
							, new CSpecificMediumFilter( context< CEnterNetworkAction >().getNodePtr() ) ) );

			price = enteranceTerms.m_price;
			return transit< CAskForAddmision >();
		}
		return discard_event();
	}

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		assert( !"no response" );
		context< CEnterNetworkAction >().forgetRequests();
		context< CEnterNetworkAction >().setExit();
		return discard_event();
	}

	boost::statechart::result react( common::CAckEvent const & _ackEvent )
	{
		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CTimeEvent >,
	boost::statechart::custom_reaction< common::CMessageResult >,
	boost::statechart::custom_reaction< common::CAckEvent >
	> reactions;
};


struct CAskForAddmision : boost::statechart::state< CAskForAddmision, CEnterNetworkAction >
{
	CAskForAddmision( my_context ctx ): my_base( ctx )
	{
		common::CSendMessageRequest * request =
				new common::CSendMessageRequest(
					common::CPayloadKind::EnterNetworkAsk
					, context< CEnterNetworkAction >().getActionKey()
					, new CMediumClassFilter( common::CMediumKinds::Monitors, 1 ) );

		request->addPayload( common::CAdmitAsk() );

		context< CEnterNetworkAction >().addRequest( request );

		context< CEnterNetworkAction >().addRequest(
					new common::CTimeEventRequest(
						 WaitTime
						, new CMediumClassFilter( common::CMediumKinds::Time ) ) );
	}

	boost::statechart::result react( common::CMessageResult const & _messageResult )
	{
		common::CMessage orginalMessage;
		if ( !common::CommunicationProtocol::unwindMessage( _messageResult.m_message, orginalMessage, GetTime(), _messageResult.m_pubKey ) )
			assert( !"service it somehow" );

		if ( _messageResult.m_message.m_header.m_payloadKind == common::CPayloadKind::Result )
		{
			common::CResult result;

			common::convertPayload( orginalMessage, result );

			context< CEnterNetworkAction >().addRequest(
						new common::CAckRequest(
							context< CEnterNetworkAction >().getActionKey()
							, _messageResult.m_message.m_header.m_id
							, new CSpecificMediumFilter( context< CEnterNetworkAction >().getNodePtr() ) ) );

			if ( result.m_result )
			{
				monitorKey = _messageResult.m_pubKey;
				return price ? transit< CNetworkAlive >() : transit< CFreeRegistration >();
			}
			else
			{
				assert(!"don't want cooperate");
			}

		}
		return discard_event();
	}

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		// not  responding  do  something
		context< CEnterNetworkAction >().setExit();
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

struct CFreeRegistrationEnter : boost::statechart::state< CFreeRegistrationEnter, CEnterNetworkAction >
{
	CFreeRegistrationEnter( my_context ctx )
		: my_base( ctx )
	{}
};

struct CNetworkAlive : boost::statechart::state< CNetworkAlive, CEnterNetworkAction >
{
	CNetworkAlive( my_context ctx )
		: my_base( ctx )
	{
		context< CEnterNetworkAction >().addRequest(
					new common::CScheduleActionRequest(
						new CPassTransactionAction(
							monitorKey.GetID()
							, price )
						, new CMediumClassFilter( common::CMediumKinds::Schedule) ) );
	}

	boost::statechart::result react( common::CMessageResult const & _messageResult )
	{
		common::CMessage orginalMessage;
		if ( !common::CommunicationProtocol::unwindMessage( _messageResult.m_message, orginalMessage, GetTime(), _messageResult.m_pubKey ) )
			assert( !"service it somehow" );

		common::CResult result;

		common::convertPayload( orginalMessage, result );

		assert( result.m_result );// for debug only, do something here

		common::CSendMessageRequest * request =
				new common::CSendMessageRequest(
					common::CPayloadKind::Ack
					, context< CEnterNetworkAction >().getActionKey()
					, _messageResult.m_message.m_header.m_id
					, new CSpecificMediumFilter( context< CEnterNetworkAction >().getNodePtr() ) );

		request->addPayload( common::CAck() );

		context< CEnterNetworkAction >().addRequest( request );

		context< CEnterNetworkAction >().setExit();

		return discard_event();
	}

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		context< CEnterNetworkAction >().setExit();

		return discard_event();
	}

	boost::statechart::result react( common::CTransactionAckEvent const & _transactionAckEvent )
	{
		if ( _transactionAckEvent.m_status == common::TransactionsStatus::Invalid )
		{
			context< CEnterNetworkAction >().setExit();
		}
		else
		{
			common::CAdmitProof admitProof;
			admitProof.m_proofTransactionHash = _transactionAckEvent.m_transactionSend.GetHash();

			common::CSendMessageRequest * request =
					new common::CSendMessageRequest(
						common::CPayloadKind::AdmitProof
						, context< CEnterNetworkAction >().getActionKey()
						, new CSpecificMediumFilter( context< CEnterNetworkAction >().getNodePtr() ) );

			request->addPayload(admitProof);

			context< CEnterNetworkAction >().addRequest( request );

			context< CEnterNetworkAction >().addRequest(
						new common::CTimeEventRequest(
							 WaitTime
							, new CMediumClassFilter( common::CMediumKinds::Time ) ) );

		}
		return discard_event();
	}

	boost::statechart::result react( common::CAckEvent const & _ackEvent )
	{
		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CTimeEvent >,
	boost::statechart::custom_reaction< common::CAckEvent >,
	boost::statechart::custom_reaction< common::CTransactionAckEvent >,
	boost::statechart::custom_reaction< common::CMessageResult >
	> reactions;
};


struct CSynchronization : boost::statechart::state< CSynchronization, CEnterNetworkAction >
{
	CSynchronization( my_context ctx )
		: my_base( ctx )
	{
		context< CEnterNetworkAction >().forgetRequests();
		context< CEnterNetworkAction >().addRequest(
		 new common::CTimeEventRequest(
						WaitTime
						, new CMediumClassFilter( common::CMediumKinds::Time ) ) );

		context< CEnterNetworkAction >().addRequest(
					new common::CScheduleActionRequest(
						new CSynchronizationAction( context< CEnterNetworkAction >().getNodePtr() )
						, new CMediumClassFilter( common::CMediumKinds::Schedule) ) );
	}

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		context< CEnterNetworkAction >().setExit();
		return discard_event();
	}

	boost::statechart::result react( common::CAckEvent const & _ackEvent )
	{
		return discard_event();
	}

	boost::statechart::result react( common::CSynchronizationResult const & _synchronizationResult )
	{
		if ( _synchronizationResult.m_result )
		{
	//		return transit< CFetchRankingTimeAndInfo >();
		}

		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CSynchronizationResult >,
	boost::statechart::custom_reaction< common::CTimeEvent >,
	boost::statechart::custom_reaction< common::CAckEvent >
	> reactions;
};

struct CFetchRankingTimeAndInfo : boost::statechart::state< CFetchRankingTimeAndInfo, CEnterNetworkAction >
{
	CFetchRankingTimeAndInfo( my_context ctx ): my_base( ctx )
	{
		common::CSendMessageRequest * request =
				new common::CSendMessageRequest(
					common::CPayloadKind::InfoReq
					, context< CEnterNetworkAction >().getActionKey()
					, new CSpecificMediumFilter( context< CEnterNetworkAction >().getNodePtr() ) );

				request->addPayload( (int)common::CInfoKind::StorageInfoAsk, std::vector<unsigned char>() );

				context< CEnterNetworkAction >().addRequest( request );

		context< CEnterNetworkAction >().addRequest(
					new common::CTimeEventRequest(
						WaitTime
						, new CMediumClassFilter( common::CMediumKinds::Time ) ) );
	}
	boost::statechart::result react( common::CMessageResult const & _messageResult )
	{
		common::CMessage orginalMessage;
		if ( !common::CommunicationProtocol::unwindMessage( _messageResult.m_message, orginalMessage, GetTime(), _messageResult.m_pubKey ) )
			assert( !"service it somehow" );

		common::CRankingInfo rankingInfo;

		common::convertPayload( orginalMessage, rankingInfo );

		common::CSendMessageRequest * request =
				new common::CSendMessageRequest(
					common::CPayloadKind::Ack
					, context< CEnterNetworkAction >().getActionKey()
					, _messageResult.m_message.m_header.m_id
					, new CSpecificMediumFilter( context< CEnterNetworkAction >().getNodePtr() ) );

		return discard_event();
	}

	boost::statechart::result react( common::CTimeEvent const & _timeEvent ){}

	boost::statechart::result react( common::CAckEvent const & _ackEvent ){}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CAckEvent >,
	boost::statechart::custom_reaction< common::CTimeEvent >,
	boost::statechart::custom_reaction< common::CMessageResult >
	> reactions;
};

struct CSendRankingTimeAndInfo : boost::statechart::state< CSendRankingTimeAndInfo, CEnterNetworkAction >
{
	CSendRankingTimeAndInfo( my_context ctx ): my_base( ctx )
	{
		common::CSendMessageRequest * request =
				new common::CSendMessageRequest(
					common::CPayloadKind::RankingInfo
					, context< CEnterNetworkAction >().getActionKey()
					, new CSpecificMediumFilter( context< CEnterNetworkAction >().getNodePtr() ) );

		common::CRankingInfo rankingInfo(
					CReputationTracker::getInstance()->getAllyTrackers()
					,CReputationTracker::getInstance()->getAllyMonitors()
					,CReputationTracker::getInstance()->getTrackers() );

		request->addPayload( rankingInfo );

		context< CEnterNetworkAction >().addRequest( request );

		context< CEnterNetworkAction >().addRequest(
					new common::CTimeEventRequest(
						WaitTime
						, new CMediumClassFilter( common::CMediumKinds::Time ) ) );

	}
	boost::statechart::result react( common::CMessageResult const & _messageResult )
	{
		common::CMessage orginalMessage;
		if ( !common::CommunicationProtocol::unwindMessage( _messageResult.m_message, orginalMessage, GetTime(), _messageResult.m_pubKey ) )
			assert( !"service it somehow" );

		if ( orginalMessage.m_header.m_payloadKind == common::CPayloadKind::InfoReq )
		{
			common::CInfoRequestData infoRequest;

			common::convertPayload( orginalMessage, infoRequest );

			if ( infoRequest.m_kind == common::CInfoKind::RankingAsk )
			{

				common::CSendMessageRequest * request =
						new common::CSendMessageRequest(
							common::CPayloadKind::Ack
							, context< CEnterNetworkAction >().getActionKey()
							, _messageResult.m_message.m_header.m_id
							, new CSpecificMediumFilter( context< CEnterNetworkAction >().getNodePtr() ) );
			}
		}
		return discard_event();
	}
	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		context< CEnterNetworkAction >().setExit();
	}
	boost::statechart::result react( common::CAckEvent const & _ackEvent )
	{
		context< CEnterNetworkAction >().setExit();
	}
	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CAckEvent >,
	boost::statechart::custom_reaction< common::CTimeEvent >,
	boost::statechart::custom_reaction< common::CMessageResult >
	> reactions;
};

CEnterNetworkAction::CEnterNetworkAction( uintptr_t _nodePtr )
	: m_nodePtr( _nodePtr )
{
	initiate();
	process_event( CSwitchToEnter() );
}

CEnterNetworkAction::CEnterNetworkAction( uint256 const & _actionKey )
	: common::CAction( _actionKey )
{
	initiate();
	process_event( CSwitchToAdmit() );
}

void
CEnterNetworkAction::accept( common::CSetResponseVisitor & _visitor )
{
	_visitor.visit( *this );
}

}
