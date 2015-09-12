// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/statechart/state.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/custom_reaction.hpp>
#include <boost/statechart/simple_state.hpp>

#include "wallet.h"

#include "common/setResponseVisitor.h"
#include "common/commonRequests.h"
#include "common/analyseTransaction.h"

#include "tracker/clientRequestsManager.h"
#include "tracker/getBalanceAction.h"
#include "tracker/requests.h"

namespace tracker
{
unsigned int const LoopTime = 10000;

struct CFindSelf : boost::statechart::event< CFindSelf >{};
struct CFindClient : boost::statechart::event< CFindClient >{};
struct CFindBalance;
struct CGetSelfBalance;

struct CUninitiatedBalance : boost::statechart::simple_state< CUninitiatedBalance, CGetBalanceAction >
{
	typedef boost::mpl::list<
	boost::statechart::transition< CFindSelf, CGetSelfBalance >,
	boost::statechart::transition< CFindClient, CFindBalance >
	> reactions;
};

struct CFindBalance : boost::statechart::state< CFindBalance, CGetBalanceAction >
{
	CFindBalance( my_context ctx ) : my_base( ctx )
	{
		context< CGetBalanceAction >().forgetRequests();

		context< CGetBalanceAction >().addRequest(
					new CGetBalanceRequest( context< CGetBalanceAction >().getKeyId() ) );
	}

	boost::statechart::result react( common::CAvailableCoinsEvent const & _availableCoins )
	{
		CClientRequestsManager::getInstance()->setClientResponse(
					  context< CGetBalanceAction >().getHash()
					, _availableCoins );

		context< CGetBalanceAction >().setExit();

		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CAvailableCoinsEvent >
	> reactions;
};

struct CGetSelfBalance : boost::statechart::state< CGetSelfBalance, CGetBalanceAction >
{
	CGetSelfBalance( my_context ctx ) : my_base( ctx )
	{
		m_self = common::CAuthenticationProvider::getInstance()->getMyKey().GetID();

		common::CInfoAskRequest< common::CTrackerTypes > * request =
				new common::CInfoAskRequest< common::CTrackerTypes >(
					common::CInfoKind::BalanceAsk
					, context< CGetBalanceAction >().getActionKey()
					, new CMediumClassFilter( common::CMediumKinds::Trackers, 1 ) );

		request->setPayload( m_self );

		context< CGetBalanceAction >().addRequest( request );

		context< CGetBalanceAction >().addRequest(
					new common::CTimeEventRequest< common::CTrackerTypes >(
						LoopTime
						, new CMediumClassFilter( common::CMediumKinds::Time ) ) );

	}

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		assert( !"for now  asser this" );
		context< CGetBalanceAction >().setExit();
		return discard_event();
	}

	boost::statechart::result react( common::CMessageResult const & _messageResult )
	{
		common::CMessage orginalMessage;
		if ( !common::CommunicationProtocol::unwindMessage( _messageResult.m_message, orginalMessage, GetTime(), _messageResult.m_pubKey ) )
			assert( !"service it somehow" );

		if ( orginalMessage.m_header.m_payloadKind == common::CPayloadKind::Balance )
		{
			common::CBalance balance;
			common::convertPayload( orginalMessage, balance );

			context< CGetBalanceAction >().addRequest(
						new common::CAckRequest< common::CTrackerTypes >(
							context< CGetBalanceAction >().getActionKey()
							, orginalMessage.m_header.m_id
							, new CSpecificMediumFilter( _messageResult.m_nodeIndicator ) ) );

			std::map< uint256, CCoins >::const_iterator iterator = balance.m_availableCoins.begin();

			CWallet::getInstance()->replaceAvailableCoins( m_self, std::vector< CAvailableCoin >() );

			while( iterator != balance.m_availableCoins.end() )
			{
				std::vector< CAvailableCoin > availableCoins
						= common::getAvailableCoins(
							iterator->second
							, m_self
							, iterator->first );

				CWallet::getInstance()->addAvailableCoins( m_self, availableCoins );
				iterator++;
			}
			context< CGetBalanceAction >().setResult( common::CExecutedIndicator(true) );
			context< CGetBalanceAction >().setExit();
		}

		return discard_event();
	}

	boost::statechart::result react( common::CAckEvent const & _promptAck )
	{
		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CTimeEvent >,
	boost::statechart::custom_reaction< common::CMessageResult >,
	boost::statechart::custom_reaction< common::CAckEvent >
	> reactions;

	CKeyID m_self;
};

CGetBalanceAction::CGetBalanceAction()
{
	initiate();
	process_event( CFindSelf() );
}

CGetBalanceAction::CGetBalanceAction( uint160 const & _keyId, uint256 const & _hash )
	: m_keyId( _keyId )
	, m_hash( _hash )
{
	initiate();
	process_event( CFindClient() );
}

void
CGetBalanceAction::accept( common::CSetResponseVisitor< common::CTrackerTypes > & _visitor )
{
	_visitor.visit( *this );
}


}
