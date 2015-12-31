// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "common/communicationProtocol.h"
#include "common/expireRegister.h"

#include "monitor/cleanupAction.h"

namespace monitor
{
/*
check  if  those  transaction  exist

or  create  new  action

in  general don't need much minimal

and  send  transactions  to  trackers  our

*/
//common::CExpireRegister

struct CInitiateCleanup;

struct CInitiateEvent : boost::statechart::event< CInitiateEvent >{};

struct CPassOnEvent : boost::statechart::event< CPassOnEvent >{};

struct CleanupInit : boost::statechart::simple_state< CleanupInit, CCleanupAction >
{
	typedef boost::mpl::list<
	boost::statechart::transition< CInitiateEvent, CInitiateCleanup >,
	boost::statechart::transition< CPassOnEvent, CPassCleanup >
	> reactions;
};


struct CInitiateCleanup : boost::statechart::state< CInitiateCleanup, CCleanupAction >
{
	CInitiateCleanup( my_context ctx )
		: my_base( ctx )
	{
		LogPrintf("admit tracker action: %p wait for info \n", &context< CCleanupAction >() );
		context< CCleanupAction >().forgetRequests();
		context< CCleanupAction >().addRequest( new common::CTimeEventRequest( WaitTime, new CMediumClassFilter( common::CMediumKinds::Time ) ) );

	}

	boost::statechart::result react( common::CMessageResult const & _messageResult )
	{
		common::CMessage orginalMessage;
		if ( !common::CommunicationProtocol::unwindMessage( _messageResult.m_message, orginalMessage, GetTime(), _messageResult.m_pubKey ) )
			assert( !"service it somehow" );

		// todo create alredy registered logic _messageResult.m_pubKey

		if ( _messageResult.m_message.m_header.m_payloadKind == common::CPayloadKind::TransactionsCleanup )
		{
				common::CCleanupMessage cleanupMessage;
						  common::convertPayload( orginalMessage, cleanupMessage );

						  context< CCleanupAction >().forgetRequests();

		context< CCleanupAction >().addRequest(
					new common::CAckRequest(
						context< CCleanupAction >().getActionKey()
						, _messageResult.m_message.m_header.m_id
						, new CByKeyMediumFilter( _messageResult.m_pubKey ) ) );

		context< CCleanupAction >().addRequest(
				new common::CSendMessageRequest(
					common::CPayloadKind::Result
					, common::CResult( CController::getInstance()->isAdmitted() ?1 : 0 )
					, context< CCleanupAction >().getActionKey()
					, _messageResult.m_message.m_header.m_id
					, new CByKeyMediumFilter( _messageResult.m_pubKey ) ) );

		if ( !CController::getInstance()->isAdmitted() )
			context< CCleanupAction >().setExit();

		return discard_event();
	}

	boost::statechart::result react( common::CAckEvent const & _ackEvent )
	{
		if ( CController::getInstance()->getPrice() )
		{
			if ( CReputationTracker::getInstance()->getTrackers().empty() )
				return transit< CPaidRegistration >();
		}
		else
		{

			CAddress address;
			if ( !CReputationTracker::getInstance()->getAddresFromKey( context< CCleanupAction >().getPartnerKey().GetID(), address ) )
				assert( !"problem" );

			CReputationTracker::getInstance()->addNodeToSynch( context< CCleanupAction >().getPartnerKey().GetID() );
			common::CTrackerData	trackerData;
			if( CReputationTracker::getInstance()->getTracker( context< CCleanupAction >().getPartnerKey().GetID(), trackerData ) )
			{
				trackerData.m_networkTime = CController::getInstance()->getPeriod();
				trackerData.m_contractTime = GetTime();

			}
			else
			{
				trackerData = common::CTrackerData(
							context< CCleanupAction >().getPartnerKey()
							, address
							, 0
							, CController::getInstance()->getPeriod()
							, GetTime() );
			}

			CRankingDatabase::getInstance()->writeTrackerData( trackerData );

			CReputationTracker::getInstance()->addTracker( trackerData );
			context< CCleanupAction >().setExit();
		}

		return discard_event();
	}

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		context< CCleanupAction >().setExit();
		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CAckEvent >,
	boost::statechart::custom_reaction< common::CTimeEvent >,
	boost::statechart::custom_reaction< common::CMessageResult >
	> reactions;
};

struct CPassCleanup : boost::statechart::state< CPassCleanup, CCleanupAction >
{
	CInitiateCleanup( my_context ctx )
		: my_base( ctx )
	{
		LogPrintf("admit tracker action: %p wait for info \n", &context< CCleanupAction >() );
		context< CCleanupAction >().forgetRequests();
		context< CCleanupAction >().addRequest( new common::CTimeEventRequest( WaitTime, new CMediumClassFilter( common::CMediumKinds::Time ) ) );

	}

	boost::statechart::result react( common::CMessageResult const & _messageResult )
	{
		common::CMessage orginalMessage;
		if ( !common::CommunicationProtocol::unwindMessage( _messageResult.m_message, orginalMessage, GetTime(), _messageResult.m_pubKey ) )
			assert( !"service it somehow" );

		// todo create alredy registered logic _messageResult.m_pubKey

		if ( _messageResult.m_message.m_header.m_payloadKind == common::CPayloadKind::TransactionsCleanup )
		{
			common::CCleanupMessage cleanupMessage;
			common::convertPayload( orginalMessage, cleanupMessage );

			context< CCleanupAction >().forgetRequests();

		context< CCleanupAction >().addRequest(
					new common::CAckRequest(
						context< CCleanupAction >().getActionKey()
						, _messageResult.m_message.m_header.m_id
						, new CByKeyMediumFilter( _messageResult.m_pubKey ) ) );



		context< CCleanupAction >().addRequest(
				new common::CSendMessageRequest(
					common::CPayloadKind::Result
					, common::CResult( CController::getInstance()->isAdmitted() ?1 : 0 )
					, context< CCleanupAction >().getActionKey()
					, _messageResult.m_message.m_header.m_id
					, new CByKeyMediumFilter( _messageResult.m_pubKey ) ) );

		if ( !CController::getInstance()->isAdmitted() )
			context< CCleanupAction >().setExit();

		return discard_event();
	}

	boost::statechart::result react( common::CAckEvent const & _ackEvent )
	{
		if ( CController::getInstance()->getPrice() )
		{
			if ( CReputationTracker::getInstance()->getTrackers().empty() )
				return transit< CPaidRegistration >();
		}
		else
		{

			CAddress address;
			if ( !CReputationTracker::getInstance()->getAddresFromKey( context< CCleanupAction >().getPartnerKey().GetID(), address ) )
				assert( !"problem" );

			CReputationTracker::getInstance()->addNodeToSynch( context< CCleanupAction >().getPartnerKey().GetID() );
			common::CTrackerData	trackerData;
			if( CReputationTracker::getInstance()->getTracker( context< CCleanupAction >().getPartnerKey().GetID(), trackerData ) )
			{
				trackerData.m_networkTime = CController::getInstance()->getPeriod();
				trackerData.m_contractTime = GetTime();

			}
			else
			{
				trackerData = common::CTrackerData(
							context< CCleanupAction >().getPartnerKey()
							, address
							, 0
							, CController::getInstance()->getPeriod()
							, GetTime() );
			}

			CRankingDatabase::getInstance()->writeTrackerData( trackerData );

			CReputationTracker::getInstance()->addTracker( trackerData );
			context< CCleanupAction >().setExit();
		}

		return discard_event();
	}

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		context< CCleanupAction >().setExit();
		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CAckEvent >,
	boost::statechart::custom_reaction< common::CTimeEvent >,
	boost::statechart::custom_reaction< common::CMessageResult >
	> reactions;

};
CCleanupAction::CCleanupAction()
{
}

void
CCleanupAction::accept( common::CSetResponseVisitor & _visitor )
{
	_visitor.visit( *this );
}

}
