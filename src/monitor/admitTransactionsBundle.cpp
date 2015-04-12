// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/statechart/state.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/custom_reaction.hpp>

#include "common/setResponseVisitor.h"

#include "monitor/admitTransactionsBundle.h"

namespace monitor
{

struct CWaitForBundle : boost::statechart::state< CWaitForBundle, CAdmitProofTransactionBundle >
{
	CWaitForBundle( my_context ctx ) : my_base( ctx )
	{
	}

	boost::statechart::result react( common::CMessageResult const & _messageResult )
	{
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CMessageResult >
	> reactions;
};

CAdmitProofTransactionBundle::CAdmitProofTransactionBundle()
	: common::CAction< common::CMonitorTypes >( false )
	, CCommunicationAction( getActionKey() )
{
	initiate();
}

void
CAdmitProofTransactionBundle::accept( common::CSetResponseVisitor< common::CMonitorTypes > & _visitor )
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
