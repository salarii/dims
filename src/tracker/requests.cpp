// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "requests.h"
#include "common/mediumKinds.h"
#include "filters.h"

namespace tracker
{

CValidateTransactionsRequest::CValidateTransactionsRequest( std::vector< CTransaction > const & _transactions, common::CMediumFilter * _mediumFilter )
	: common::CRequest( _mediumFilter )
	, m_transactions( _transactions )
{
}

void CValidateTransactionsRequest::accept( common::CMedium * _medium ) const
{
	_medium->add( this );
}

std::vector< CTransaction > const &
CValidateTransactionsRequest::getTransactions() const
{
	return m_transactions;
}

CPassMessageRequest::CPassMessageRequest(  common::CMessage const & _message, uint256 const & _actionKey, CPubKey const & _prevKey, common::CMediumFilter * _mediumFilter )
	: common::CRequest( _mediumFilter )
	, m_actionKey( _actionKey )
	, m_message( _message )
	, m_prevKey( _prevKey )
{
}

void
CPassMessageRequest::accept( common::CMedium * _medium ) const
{
	_medium->add( this );
}

common::CMessage const &
CPassMessageRequest::getMessage() const
{
	return m_message;
}

uint256
CPassMessageRequest::getActionKey() const
{
	return m_actionKey;
}

CPubKey const &
CPassMessageRequest::getPreviousKey() const
{
	return m_prevKey;
}

CConnectToTrackerRequest::CConnectToTrackerRequest( std::string const & _trackerAddress, CAddress const & _serviceAddress )
	: common::CRequest( new CMediumClassFilter( common::CMediumKinds::Internal ) )
	, m_trackerAddress( _trackerAddress )
	, m_serviceAddress( _serviceAddress )
{
}

void
CConnectToTrackerRequest::accept( common::CMedium * _medium ) const
{
	_medium->add( this );
}
std::string
CConnectToTrackerRequest::getAddress() const
{
	return m_trackerAddress;
}

CAddress
CConnectToTrackerRequest::getServiceAddress() const
{
	return m_serviceAddress;
}

CGetBalanceRequest::CGetBalanceRequest( uint160 const & _key )
	: common::CRequest( new CMediumClassFilter( common::CMediumKinds::Internal ) )
	, m_key( _key )
{
}

void
CGetBalanceRequest::accept( common::CMedium * _medium ) const
{
	_medium->add( this );
}

uint160
CGetBalanceRequest::getKey() const
{
	return m_key;
}

}
