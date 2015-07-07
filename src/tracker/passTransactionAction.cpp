// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/state.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/custom_reaction.hpp>

#include "tracker/passTransactionAction.h"
#include "tracker/trackerFilters.h"

namespace tracker
{

struct CProcessAsClient;
struct CValidInNetwork;

unsigned int const LoopTime = 10000;

struct CValidInNetworkEvent : boost::statechart::event< CValidInNetworkEvent >
{
};

struct CInvalidInNetworkEvent : boost::statechart::event< CInvalidInNetworkEvent >
{
};

struct CInitial : boost::statechart::simple_state< CInitial, CPassTransactionAction >
{
	typedef boost::mpl::list<
	boost::statechart::transition< CValidInNetworkEvent, CValidInNetwork >,
	boost::statechart::transition< CInvalidInNetworkEvent, CProcessAsClient >
	> reactions;
};

struct CValidInNetwork : boost::statechart::state< CValidInNetwork, CPassTransactionAction >
{
	CValidInNetwork( my_context ctx ) : my_base( ctx )
	{
		transaction = context< CPassTransactionAction >().getTransaction();
		CTransactionRecordManager::getInstance()->addClientTransaction( transaction );

				context< CPassTransactionAction >().addRequest(
							new common::CTimeEventRequest< common::CTrackerTypes >(
								  LoopTime
								, new CMediumClassFilter( common::CMediumKinds::Time ) ) );
	}

	// pool if  given  transaction  executed ????
	// pool transaction status

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		if ( CTransactionRecordManager::getInstance()->getTransaction( _transactionStatus.m_hash, transaction ) )
		{
			context< CPassTransactionAction >().setResult( transaction );
		}
		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CTimeEvent >
	> reactions;

	CTransaction transaction;
};

struct CProcessAsClient : boost::statechart::state< CProcessAsClient, CPassTransactionAction >
{
	CProcessAsClient()
	{
		context< CPassTransactionAction >().addRequest(
					new CTransactionAsClientRequest(
						  context< CPassTransactionAction >().getTransaction()
						, context< CPassTransactionAction >().getActionKey()
						, new CMediumClassFilter( common::CMediumKinds::Trackers, 1 ) ) );
	}

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		return discard_event();
	}

	boost::statechart::result react( common::CMessageResult const & _messageResult )
	{
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

CPassTransactionAction::CPassTransactionAction( CTransaction const & _transaction )
	: m_transaction( _transaction )
{
	initiate();
}

void
CPassTransactionAction::accept( common::CSetResponseVisitor< common::CTrackerTypes > & _visitor )
{
	_visitor.visit( *this );
}

}
