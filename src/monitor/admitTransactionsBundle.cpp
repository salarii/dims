// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/statechart/state.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/custom_reaction.hpp>

#include "common/setResponseVisitor.h"
#include "common/analyseTransaction.h"

#include "monitor/admitTransactionsBundle.h"
#include "monitor/reputationTracer.h"
#include "monitor/transactionRecordManager.h"
#include "monitor/filters.h"
#include "monitor/controller.h"
#include "monitor/chargeRegister.h"

namespace monitor
{
// TODO: bundle  should  be  propagated to synchronizing  at  the moment  nodes
unsigned const InvestigationStartTime = 60000;

struct CWaitForBundle : boost::statechart::state< CWaitForBundle, CAdmitTransactionBundle >
{
	CWaitForBundle( my_context ctx ) : my_base( ctx )
	{
		context< CAdmitTransactionBundle >().addRequest(
					new common::CTimeEventRequest(
						InvestigationStartTime
						, new CMediumClassFilter( common::CMediumKinds::Time ) ) );

		m_presentTrackers = CReputationTracker::getInstance()->getPresentTrackers();
		assert( !m_presentTrackers.empty() );
	}

	boost::statechart::result react( common::CMessageResult const & _messageResult )
	{
		common::CMessage orginalMessage;

		 std::set< CPubKey > participants;
		if ( !common::CommunicationProtocol::unwindMessageAndParticipants( _messageResult.m_message, orginalMessage, GetTime(), _messageResult.m_pubKey, participants ) )
			assert( !"service it somehow" );

		if ( orginalMessage.m_header.m_payloadKind == common::CPayloadKind::Transactions )
		{
			context< CAdmitTransactionBundle >().addRequest(
						new common::CAckRequest(
							context< CAdmitTransactionBundle >().getActionKey()
							, orginalMessage.m_header.m_id
							, new CByKeyMediumFilter( _messageResult.m_pubKey ) ) );

			common::CTransactionBundle transactionBundle;

			common::convertPayload( orginalMessage, transactionBundle );

			BOOST_FOREACH( CPubKey const & pubKey, participants )
			{
				m_presentTrackers.erase( pubKey.GetID() );
			}

			context< CAdmitTransactionBundle >().addRequest(
					new common::CSendMessageRequest(
							_messageResult.m_message
						, _messageResult.m_pubKey
						, context< CAdmitTransactionBundle >().getActionKey()
						, new CMediumClassFilter( common::CMediumKinds::Monitors ) ) );

			// TODO: send  transaction  to  synchronizaing nodes

			if ( m_presentTrackers.empty() )
			{
				// if  registration  in  progress  those  should  be  stored
				if ( CChargeRegister::getInstance()->getStoreTransactions() )
				{
					CChargeRegister::getInstance()->storeTransactions( transactionBundle.m_transactions );
				}

				context< CAdmitTransactionBundle >().setExit();

				CTransactionRecordManager::getInstance()->addTransactionsToStorage( transactionBundle.m_transactions );

				//here send those transactions to synchronizing nodes
			}

			BOOST_FOREACH( CTransaction const & transaction, transactionBundle.m_transactions )
			{
				common::findSelfCoinsAndAddToWallet( transaction );
			}
		}

		return discard_event();
	}

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		assert( !"do  something with it" );
		context< CAdmitTransactionBundle >().setExit();
		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CTimeEvent >,
	boost::statechart::custom_reaction< common::CMessageResult >
	> reactions;

	std::set< uint160 > m_presentTrackers;
};

CAdmitTransactionBundle::CAdmitTransactionBundle( uint256 const & _actionKey )
	: common::CAction( _actionKey )
{
	LogPrintf("admit transaction bundle action: %p \n", this );

	initiate();
}

void
CAdmitTransactionBundle::accept( common::CSetResponseVisitor & _visitor )
{
	_visitor.visit( *this );
}

}
