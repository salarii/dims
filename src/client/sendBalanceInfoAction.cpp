// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/statechart/transition.hpp>
#include <boost/statechart/state.hpp>
#include <boost/statechart/custom_reaction.hpp>

#include "common/setResponseVisitor.h"
#include "common/commonEvents.h"
#include "common/analyseTransaction.h"

#include "sendInfoRequestAction.h"
#include "sendBalanceInfoAction.h"

#include "clientFilters.h"
#include "clientControl.h"
#include "clientEvents.h"
#include "clientRequests.h"

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
			context< CSendBalanceInfoAction >().clearRequests();
			context< CSendBalanceInfoAction >().addRequests( new CBalanceRequest( addresses.at( m_addressIndex++ ) ) );
		}
		else
			context< CSendBalanceInfoAction >().clearRequests();
	}
	// imporant  how  many trackers  service  this
	// here I assume  that  one. ??? is this correct ???
	boost::statechart::result react( CCoinsEvent const & _coinsEvent )
	{

		CMnemonicAddress address;
		address.SetString( m_pubKey );

		CKeyID keyId;
		address.GetKeyID( keyId );

		std::vector< CAvailableCoin > availableCoins;
		BOOST_FOREACH( PAIRTYPE( uint256, CCoins ) const & coins, _coinsEvent.m_coins )
		{
			std::vector< CAvailableCoin > tempCoins = common::getAvailableCoins( coins.second, keyId, coins.first );
			availableCoins.insert(availableCoins.end(), tempCoins.begin(), tempCoins.end());
		}
		CWallet::getInstance()->replaceAvailableCoins( keyId, availableCoins );

		std::vector< std::string > const & m_addresses = context< CSendBalanceInfoAction >().getAddresses();

		updateTotal( availableCoins );

		if ( m_addressIndex < m_addresses.size() )
		{
			m_pubKey = m_addresses.at( m_addressIndex );
			context< CSendBalanceInfoAction >().clearRequests();
			context< CSendBalanceInfoAction >().addRequests( new CBalanceRequest( m_addresses.at( m_addressIndex++ ) ) );
		}
		else
		{
			CClientControl::getInstance()->updateTotalBalance( m_total );
			context< CSendBalanceInfoAction >().clearRequests();
		}
		return discard_event();
	}

	boost::statechart::result react( common::CPending const & _pending )
	{
		context< CSendBalanceInfoAction >().clearRequests();
		context< CSendBalanceInfoAction >().addRequests( new CInfoRequestContinue( _pending.m_token, new CSpecificMediumFilter( _pending.m_networkPtr ) ) );
		return discard_event();
	}

	boost::statechart::result react( common::CNoMedium const & _noMedium )
	{
		context< CSendBalanceInfoAction >().clearRequests();
		return discard_event();
	}

	void updateTotal( std::vector< CAvailableCoin > const & _availableCoin )
	{
		BOOST_FOREACH( CAvailableCoin const & coin, _availableCoin )
		{
			m_total += coin.m_coin.nValue;
		}
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CPending >,
	boost::statechart::custom_reaction< common::CNoMedium >,
	boost::statechart::custom_reaction< CCoinsEvent >
	> reactions;

	unsigned int m_addressIndex;

	std::set< uintptr_t > m_pending;

	std::map< uintptr_t, uint256 > m_nodeToToken;

	std::string m_pubKey;

	int64_t m_total;
};

CSendBalanceInfoAction::CSendBalanceInfoAction( bool _autoDelete )
	: common::CAction< NodeResponses >( _autoDelete )
{
	initiate();
}

void
CSendBalanceInfoAction::accept( common::CSetResponseVisitor< NodeResponses > & _visitor )
{
	_visitor.visit( *this );
}

void
CSendBalanceInfoAction::reset()
{
	common::CAction< NodeResponses >::reset();
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

CBalanceRequest::CBalanceRequest( std::string _address )
	: common::CRequest< NodeResponses >( new CMediumClassFilter( RequestKind::Balance, 1 ) )
	, m_address( _address )
{
}

void
CBalanceRequest::accept( common::CMedium< NodeResponses > * _medium ) const
{
	_medium->add( this );
}

inline
common::CMediumFilter< NodeResponses > *
CBalanceRequest::getMediumFilter() const
{
	return common::CRequest< NodeResponses >::m_mediumFilter;
}

}
