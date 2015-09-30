// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/statechart/transition.hpp>
#include <boost/statechart/state.hpp>
#include <boost/statechart/custom_reaction.hpp>

#include "common/setResponseVisitor.h"
#include "common/events.h"
#include "common/analyseTransaction.h"
#include "common/requests.h"

#include "sendInfoRequestAction.h"
#include "sendBalanceInfoAction.h"

#include "client/filters.h"
#include "client/control.h"
#include "client/events.h"
#include "client/requests.h"

#include "serialize.h"
#include "base58.h"
#include "common/nodeMessages.h"

#include "wallet.h"

using namespace  common;

namespace client
{

struct CGetBalanceInfo : boost::statechart::state< CGetBalanceInfo, CSendBalanceInfoAction >
{
	CGetBalanceInfo( my_context ctx ) : my_base( ctx ), m_total( 0 )
	{
		std::vector< std::string > const & addresses = CClientControl::getInstance()->getAvailableAddresses();

		context< CSendBalanceInfoAction >().setAddresses( addresses );

		m_addressIndex = 0;

		if ( m_addressIndex < addresses.size() )
		{
			m_pubKey = addresses.at( m_addressIndex );
			context< CSendBalanceInfoAction >().forgetRequests();

			common::CSendMessageRequest * request =
					new common::CSendMessageRequest(
						common::CMainRequestType::BalanceInfoReq
						, new CMediumClassFilter( ClientMediums::Trackers, 1 ) );

			request->addPayload( CClientBalanceAsk( addresses.at( m_addressIndex++ ) ) );

			context< CSendBalanceInfoAction >().addRequest( request );
		}
		else
			context< CSendBalanceInfoAction >().setExit();
	}

	boost::statechart::result react( common::CNoMedium const & _noMedium )
	{
		context< CSendBalanceInfoAction >().forgetRequests();
		return discard_event();
	}

	void updateTotal( std::vector< CAvailableCoin > const & _availableCoin )
	{
		BOOST_FOREACH( CAvailableCoin const & coin, _availableCoin )
		{
			m_total += coin.m_coin.nValue;
		}
	}

	boost::statechart::result react( common::CClientMessageResponse const & _message )
	{
		common::CAvailableCoinsData availableCoinsData;
		convertClientPayload( _message.m_clientMessage, availableCoinsData );

		CMnemonicAddress address;
		address.SetString( m_pubKey );

		CKeyID keyId;
		address.GetKeyID( keyId );

		std::vector< CAvailableCoin > availableCoins;
		BOOST_FOREACH( PAIRTYPE( uint256, CCoins ) const & coins, availableCoinsData.m_availableCoins )
		{
			std::vector< CAvailableCoin > tempCoins = common::getAvailableCoins( coins.second, keyId, coins.first );
			availableCoins.insert(availableCoins.end(), tempCoins.begin(), tempCoins.end());
		}

		CWallet::getInstance()->addInputs( availableCoinsData.m_transactionInputs );

		CWallet::getInstance()->replaceAvailableCoins( keyId, availableCoins );

		std::vector< std::string > const & m_addresses = context< CSendBalanceInfoAction >().getAddresses();

		updateTotal( availableCoins );

		if ( m_addressIndex < m_addresses.size() )
		{
			m_pubKey = m_addresses.at( m_addressIndex );
			context< CSendBalanceInfoAction >().forgetRequests();

			common::CSendMessageRequest * request =
					new common::CSendMessageRequest(
						common::CMainRequestType::BalanceInfoReq
						, new CMediumClassFilter( ClientMediums::Trackers, 1 ) );

			request->addPayload( CClientBalanceAsk( m_addresses.at( m_addressIndex++ ) ) );

			context< CSendBalanceInfoAction >().addRequest( request );
		}
		else
		{
			CClientControl::getInstance()->updateTotalBalance( m_total );
			context< CSendBalanceInfoAction >().setExit();
		}
		return discard_event();

	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CNoMedium >,
	boost::statechart::custom_reaction< common::CClientMessageResponse >
	> reactions;

	unsigned int m_addressIndex;

	std::set< uintptr_t > m_pending;

	std::map< uintptr_t, uint256 > m_nodeToToken;

	std::string m_pubKey;

	int64_t m_total;
};

CSendBalanceInfoAction::CSendBalanceInfoAction( bool _autoDelete )
	: common::CAction( _autoDelete )
{
	initiate();
}

void
CSendBalanceInfoAction::accept( common::CSetResponseVisitor & _visitor )
{
	_visitor.visit( *this );
}

void
CSendBalanceInfoAction::reset()
{
	common::CAction::reset();
	initiate();
}

void
CSendBalanceInfoAction::setAddresses( std::vector< std::string > const & _addresses )
{
	m_addresses = _addresses;
}

std::vector< std::string > const &
CSendBalanceInfoAction::getAddresses() const
{
	return m_addresses;
}

}
