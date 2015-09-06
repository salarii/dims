// Copyright (c) 2014-2015 Dims dev-team
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
#include "monitor/monitorController.h"
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
					new common::CTimeEventRequest< common::CMonitorTypes >(
						InvestigationStartTime
						, new CMediumClassFilter( common::CMediumKinds::Time ) ) );
	}

	boost::statechart::result react( common::CMessageResult const & _messageResult )
	{
		common::CMessage orginalMessage;
		if ( !common::CommunicationProtocol::unwindMessage( _messageResult.m_message, orginalMessage, GetTime(), _messageResult.m_pubKey ) )
			assert( !"service it somehow" );

		if ( orginalMessage.m_header.m_payloadKind == common::CPayloadKind::Transactions )
		{
			context< CAdmitTransactionBundle >().addRequest(
						new common::CAckRequest< common::CMonitorTypes >(
							context< CAdmitTransactionBundle >().getActionKey()
							, orginalMessage.m_header.m_id
							, new CSpecificMediumFilter( _messageResult.m_nodeIndicator ) ) );

			common::CTransactionBundle transactionBundle;

			common::convertPayload( orginalMessage, transactionBundle );

			m_trackers.insert( _messageResult.m_nodeIndicator );

			// this  condition  is wrong  but  for now,  better  this  than nothing
			if ( m_trackers.size() == CReputationTracker::getInstance()->getTrackers().size() )
			{
				// if  registration  in  progress  those  should  be  stored
				if ( CChargeRegister::getInstance()->getStoreTransactions() )
				{
					CChargeRegister::getInstance()->storeTransactions( transactionBundle.m_transactions );
				}

				CTransactionRecordManager::getInstance()->addTransactionsToStorage( transactionBundle.m_transactions );
			}
		}

		context< CAdmitTransactionBundle >().setExit();
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

	std::set< uintptr_t > m_trackers;
};

CAdmitTransactionBundle::CAdmitTransactionBundle( uint256 const & _actionKey )
	: common::CAction< common::CMonitorTypes >( _actionKey )
{
	initiate();
}

void
CAdmitTransactionBundle::accept( common::CSetResponseVisitor< common::CMonitorTypes > & _visitor )
{
	_visitor.visit( *this );
}

}
