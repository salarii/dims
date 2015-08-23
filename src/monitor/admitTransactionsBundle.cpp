// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/statechart/state.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/custom_reaction.hpp>

#include "common/setResponseVisitor.h"

#include "monitor/admitTransactionsBundle.h"
#include "monitor/reputationTracer.h"
#include "monitor/transactionRecordManager.h"
#include "monitor/filters.h"

namespace monitor
{

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
				if ( CPaymentTracking::getInstance()->getStoreTransactions() )
				{
					CPaymentTracking::getInstance()->storeTransactions( transactionBundle.m_transactions );
				}

				CTransactionRecordManager::getInstance()->addTransactionsToStorage( transactionBundle.m_transactions );
			}
		}
		return discard_event();
	}

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		assert( !"do  something with it" );
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

CPaymentTracking * CPaymentTracking::ms_instance = NULL;

CPaymentTracking*
CPaymentTracking::getInstance()
{
	if ( !ms_instance )
	{
		ms_instance = new CPaymentTracking();
	};
	return ms_instance;
}

void
CPaymentTracking::addTransactionToSearch( uint256 const & _hash )
{
	boost::lock_guard<boost::mutex> lock( m_mutex );
	m_searchTransaction.insert( _hash );
}

bool
CPaymentTracking::transactionPresent( uint256 const & _transactionId, CTransaction & _transaction )
{
	boost::lock_guard<boost::mutex> lock( m_mutex );
	std::map< uint256, CTransaction >::const_iterator iterator = m_foundTransactions.find( _transactionId );

	if ( iterator == m_foundTransactions.end() )
		return false;

	_transaction = iterator->second;
	return true;
}

void
CPaymentTracking::analyseIncommingBundle( std::vector< CTransaction > const & _transactionBundle )
{
	boost::lock_guard<boost::mutex> lock( m_mutex );

	std::list< uint256 > remove;
	BOOST_FOREACH( CTransaction const & transaction, _transactionBundle )
	{
		std::set< uint256 >::const_iterator iterator = m_searchTransaction.find( transaction.GetHash() );
		if ( iterator != m_searchTransaction.end() )
		{
			remove.push_back( *iterator );
			m_foundTransactions.insert( std::make_pair( *iterator, transaction ) );
		}
	}
}

}
