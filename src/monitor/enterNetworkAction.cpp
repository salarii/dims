// Copyright (c) 2014-2015 DiMS dev-team
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
#include "monitor/reputationControlAction.h"
#include "monitor/provideInfoAction.h"

namespace monitor
{
struct CSynchronization;
struct CPaidEnterance;
struct CNetworkAlive;
struct CFreeRegistrationEnter;
struct CAssistAdmission;
struct CAdmissionCondition;
struct CAskForAddmision;
struct CFetchRankingTimeAndInfo;
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
							, new CByKeyMediumFilter( _messageResult.m_pubKey ) ) );

			common::CResult result;

			if (
					!CController::getInstance()->getEnterancePrice()
					|| !CReputationTracker::getInstance()->getTrackers().empty() )
			{
				result = common::CResult( 1 );
			}
			else
			{
				result = common::CResult( 0 );
			}

			context< CEnterNetworkAction >().addRequest(
						new common::CSendMessageRequest(
							common::CPayloadKind::Result
							, result
							, context< CEnterNetworkAction >().getActionKey()
							, _messageResult.m_message.m_header.m_id
							, new CByKeyMediumFilter( _messageResult.m_pubKey ) ) );

		}

		return discard_event();
	}

	boost::statechart::result react( common::CAckEvent const & _ackEvent )
	{
		if ( CController::getInstance()->getEnterancePrice() )
			return transit< CPaidEnterance >();
		else
		{
			CReputationTracker::getInstance()->addNodeToSynch( context< CEnterNetworkAction >().getPartnerKey().GetID() );
			CAddress address;
			if ( !CReputationTracker::getInstance()->getAddresFromKey( context< CEnterNetworkAction >().getPartnerKey().GetID(), address ) )
				assert( !"problem" );

			common::CAllyMonitorData monitorData( context< CEnterNetworkAction >().getPartnerKey(), address );

			CReputationTracker::getInstance()->addAllyMonitor( monitorData );

			context< CEnterNetworkAction >().setExit();
		}

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

struct CPaidEnterance : boost::statechart::state< CPaidEnterance, CEnterNetworkAction >
{
	CPaidEnterance( my_context ctx )
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

		CChargeRegister::getInstance()->addTransactionToSearch(
					admitMessage.m_proofTransactionHash
					, CTransactionCheck( _messageResult.m_pubKey.GetID(), CController::getInstance()->getEnterancePrice() ) );

		m_proofHash = admitMessage.m_proofTransactionHash;

		context< CEnterNetworkAction >().addRequest(
				new common::CSendMessageRequest(
					common::CPayloadKind::Ack
					, common::CAck()
					, context< CEnterNetworkAction >().getActionKey()
					, _messageResult.m_message.m_header.m_id
					, new CByKeyMediumFilter( _messageResult.m_pubKey ) ) );

		context< CEnterNetworkAction >().forgetRequests();
		context< CEnterNetworkAction >().addRequest(
					new common::CTimeEventRequest(
						m_checkPeriod
						, new CMediumClassFilter( common::CMediumKinds::Time ) ) );

		m_messageId = _messageResult.m_message.m_header.m_id;

		m_pubKey = _messageResult.m_pubKey;

		if ( !CReputationTracker::getInstance()->getAddresFromKey( m_pubKey.GetID(), m_address ) )
			assert( !"problem" );

		return discard_event();
	}

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		CTransaction transaction;

		if ( CChargeRegister::getInstance()->isTransactionPresent( m_proofHash ) )
		{

			context< CEnterNetworkAction >().addRequest(
					new common::CSendMessageRequest(
						common::CPayloadKind::Result
						, common::CResult( 1 )
						, context< CEnterNetworkAction >().getActionKey()
						, m_messageId
						, new CByKeyMediumFilter( m_pubKey ) ) );

			common::CAllyMonitorData monitorData( context< CEnterNetworkAction >().getPartnerKey(), m_address );

			CReputationTracker::getInstance()->addAllyMonitor( monitorData );
			CReputationTracker::getInstance()->addNodeToSynch( context< CEnterNetworkAction >().getPartnerKey().GetID() );
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

	~CPaidEnterance()
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

	CAddress m_address;
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

		context< CEnterNetworkAction >().addRequest(
				new common::CSendMessageRequest(
					common::CPayloadKind::InfoReq
					, common::CInfoRequestData( (int)common::CInfoKind::EnterConditionAsk, std::vector<unsigned char>() )
					, context< CEnterNetworkAction >().getActionKey()
					, new CByKeyMediumFilter( context< CEnterNetworkAction >().getPartnerKey() ) ) ); // bit  risky to ask  this way
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
							, new CByKeyMediumFilter( _messageResult.m_pubKey ) ) );

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
		context< CEnterNetworkAction >().addRequest(
				new common::CSendMessageRequest(
					common::CPayloadKind::EnterNetworkAsk
					, common::CAdmitAsk()
					, context< CEnterNetworkAction >().getActionKey()
					, new CByKeyMediumFilter( context< CEnterNetworkAction >().getPartnerKey() ) ) );

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
							, new CByKeyMediumFilter( _messageResult.m_pubKey ) ) );

			if ( result.m_result )
			{
				monitorKey = _messageResult.m_pubKey;
				return price ? transit< CNetworkAlive >() : transit< CSynchronization >();
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

struct CNetworkAlive : boost::statechart::state< CNetworkAlive, CEnterNetworkAction >
{
	CNetworkAlive( my_context ctx )
		: my_base( ctx )
	{
		context< CEnterNetworkAction >().forgetRequests();

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

		context< CEnterNetworkAction >().addRequest(
				new common::CSendMessageRequest(
					common::CPayloadKind::Ack
					, common::CAck()
					, context< CEnterNetworkAction >().getActionKey()
					, _messageResult.m_message.m_header.m_id
					, new CByKeyMediumFilter( _messageResult.m_pubKey ) ) );

		if ( result.m_result )
			return transit< CSynchronization >();

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

			context< CEnterNetworkAction >().addRequest(
					new common::CSendMessageRequest(
						common::CPayloadKind::AdmitProof
						, admitProof
						, context< CEnterNetworkAction >().getActionKey()
						, new CByKeyMediumFilter( context< CEnterNetworkAction >().getPartnerKey() ) ) );

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

	boost::statechart::result react( common::CFailureEvent const & _failureEvent )
	{
		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CTimeEvent >,
	boost::statechart::custom_reaction< common::CAckEvent >,
	boost::statechart::custom_reaction< common::CTransactionAckEvent >,
	boost::statechart::custom_reaction< common::CMessageResult >,
	boost::statechart::custom_reaction< common::CFailureEvent >
	> reactions;
};


struct CSynchronization : boost::statechart::state< CSynchronization, CEnterNetworkAction >
{
	CSynchronization( my_context ctx )
		: my_base( ctx )
	{
		context< CEnterNetworkAction >().forgetRequests();

		context< CEnterNetworkAction >().addRequest(
					new common::CScheduleActionRequest(
						new CSynchronizationAction( context< CEnterNetworkAction >().getPartnerKey() )
						, new CMediumClassFilter( common::CMediumKinds::Schedule) ) );
	}

	boost::statechart::result react( common::CAckEvent const & _ackEvent )
	{
		return discard_event();
	}

	boost::statechart::result react( common::CSynchronizationResult const & _synchronizationResult )
	{
		if ( _synchronizationResult.m_result )
		{
			return transit< CFetchRankingTimeAndInfo >();
		}
		else
			context< CEnterNetworkAction >().setExit();
		return discard_event();
	}

	boost::statechart::result react( common::CFailureEvent const & _failureEvent )
	{
		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CSynchronizationResult >,
	boost::statechart::custom_reaction< common::CAckEvent >,
	boost::statechart::custom_reaction< common::CFailureEvent >
	> reactions;
};

struct CFetchRankingTimeAndInfo : boost::statechart::state< CFetchRankingTimeAndInfo, CEnterNetworkAction >
{
	CFetchRankingTimeAndInfo( my_context ctx ): my_base( ctx )
	{
		context< CEnterNetworkAction >().addRequest(
					new common::CScheduleActionRequest(
						new CProvideInfoAction(
							common::CInfoKind::RankingFullInfo
							, context< CEnterNetworkAction >().getPartnerKey() )
						, new CMediumClassFilter( common::CMediumKinds::Schedule) ) );
	}

	boost::statechart::result react( common::CRankingEvent const & _rankingEvent )
	{
		CReputationTracker::getInstance()->clearRankingData();
		CAddress address;
		if ( !CReputationTracker::getInstance()->getAddresFromKey( context< CEnterNetworkAction >().getPartnerKey().GetID(), address ) )
			assert( !"problem" );

		common::CAllyMonitorData monitorData( context< CEnterNetworkAction >().getPartnerKey(), address );

		CReputationTracker::getInstance()->addAllyMonitor( monitorData );

		BOOST_FOREACH( common::CAllyTrackerData const & trackerData, _rankingEvent.m_rankingInfo.m_allyTrackers )
		{
			CReputationTracker::getInstance()->addAllyTracker( trackerData );
		}

		BOOST_FOREACH( common::CAllyMonitorData const & monitorData, _rankingEvent.m_rankingInfo.m_allyMonitors )
		{
			CReputationTracker::getInstance()->addAllyMonitor( monitorData );
		}

		BOOST_FOREACH( common::CTrackerData const & trackerData, _rankingEvent.m_rankingInfo.m_trackers )
		{
			CReputationTracker::getInstance()->addAllyTracker( common::CAllyTrackerData( trackerData, context< CEnterNetworkAction >().getPartnerKey() ) );
		}

		CReputationTracker::getInstance()->setMeasureReputationTime( _rankingEvent.m_rankingInfo.m_time );

		CReputationControlAction::createInstance( _rankingEvent.m_rankingInfo.m_leadingKey );
		return discard_event();
	}

	boost::statechart::result react( common::CFailureEvent const & )
	{
		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CFailureEvent >,
	boost::statechart::custom_reaction< common::CRankingEvent >
	> reactions;
};

CEnterNetworkAction::CEnterNetworkAction( CPubKey const & _partnerKey )
	: m_partnerKey( _partnerKey )
{
	initiate();
	process_event( CSwitchToEnter() );
}

CEnterNetworkAction::CEnterNetworkAction( uint256 const & _actionKey, CPubKey const & _partnerKey )
	: common::CAction( _actionKey )
	, m_partnerKey( _partnerKey )
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
