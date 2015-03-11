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
#include "common/mediumRequests.h"

#include "monitor/admitTrackerAction.h"
#include "monitor/monitorController.h"
#include "monitor/admitTransactionsBundle.h"
#include "monitor/filters.h"
#include "monitor/reputationTracer.h"

namespace monitor
{

bool analyseTransaction( CTransaction & _transaction, uint256 const & _hash, CKeyID const & _trackerId )
{
	if ( !common::findKeyInInputs( _transaction, _trackerId ) )
		return false;

	CTxOut txOut;
	unsigned id;

	bool outputExist = common::findOutputInTransaction(
				_transaction
				, common::CAuthenticationProvider::getInstance()->getMyKey().GetID()
				, txOut
				, id );

	return txOut.nValue >= CMonitorController::getInstance()->getPrice();
}

struct CWaitForInfo : boost::statechart::state< CWaitForInfo, CAdmitTrackerAction >
{
	CWaitForInfo( my_context ctx )
		: my_base( ctx )
		, m_checkPeriod( 3000 )
	{
	}

	boost::statechart::result react( common::CMessageResult const & _messageResult )
	{
		common::CMessage orginalMessage;
		if ( !common::CommunicationProtocol::unwindMessage( _messageResult.m_message, orginalMessage, GetTime(), _messageResult.m_pubKey ) )
			assert( !"service it somehow" );

		common::CAdmitAsk admitMessage;

		common::convertPayload( orginalMessage, admitMessage );

		CPaymentTracking::getInstance()->addTransactionToSearch( admitMessage.m_proofTransactionHash );

		m_proofHash = admitMessage.m_proofTransactionHash;

		context< CAdmitTrackerAction >().dropRequests();
		context< CAdmitTrackerAction >().addRequests( new common::CTimeEventRequest< common::CMonitorTypes >( m_checkPeriod, new CMediumClassFilter( common::CMediumKinds::Time ) ) );

		return discard_event();
	}

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		CTransaction transaction;

		if ( CPaymentTracking::getInstance()->transactionPresent( m_proofHash, transaction ) )
			return discard_event();

		CPubKey pubKey;
		CReputationTracker::getInstance()->getNodeToKey(
					  context< CAdmitTrackerAction >().getMedium()
					, pubKey );

		if ( analyseTransaction( transaction, m_proofHash, pubKey.GetID() ) )
		{
			//CReputationTracker::getInstance()->addTracker( CTrackerData( context< CConnectNodeAction >().getServiceAddress(), 0, context< CConnectNodeAction >().getPublicKey(), CMonitorController::getInstance()->getPeriod(), GetTime() ) );
			context< CAdmitTrackerAction >().dropRequests();
			context< CAdmitTrackerAction >().addRequests(
						new common::CResultRequest< common::CMonitorTypes >(
							  context< CAdmitTrackerAction >().getActionKey()
							, 1
							, new CSpecificMediumFilter( context< CAdmitTrackerAction >().getMedium() ) ) );

		}
		else
		{
			context< CAdmitTrackerAction >().dropRequests();
			context< CAdmitTrackerAction >().addRequests(
						new common::CResultRequest< common::CMonitorTypes >(
							  context< CAdmitTrackerAction >().getActionKey()
							, 0
							, new CSpecificMediumFilter( context< CAdmitTrackerAction >().getMedium() ) ) );

		}
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CTimeEvent >,
	boost::statechart::custom_reaction< common::CMessageResult >
	> reactions;

	uint256 m_proofHash;

	int64_t const m_checkPeriod;
};

CAdmitTrackerAction::CAdmitTrackerAction( uint256 const & _actionKey, uintptr_t _mediumPtr )
{
	initiate();
}

void
CAdmitTrackerAction::accept( common::CSetResponseVisitor< common::CMonitorTypes > & _visitor )
{
	_visitor.visit( *this );
}

}
