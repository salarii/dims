// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "validateTransactionsRequest.h"
#include "common/medium.h"
#include "common/mediumKinds.h"
#include "trackerFilters.h"

namespace tracker
{

CValidateTransactionsRequest::CValidateTransactionsRequest( std::vector< CTransaction > const & _transactions )
	: common::CRequest< TrackerResponses >( new CMediumClassFilter( common::CMediumKinds::Internal ) )
	, m_transactions( _transactions )
{
}

void CValidateTransactionsRequest::accept( common::CMedium< TrackerResponses > * _medium ) const
{
	_medium->add( this );
}

common::CMediumFilter< TrackerResponses > *
CValidateTransactionsRequest::getMediumFilter() const
{
	return common::CRequest< TrackerResponses >::m_mediumFilter;
}

std::vector< CTransaction > const &
CValidateTransactionsRequest::getTransactions() const
{
	return m_transactions;
}

CTransactionsKnownRequest::CTransactionsKnownRequest()
	: common::CRequest< TrackerResponses >( new CMediumClassFilter( common::CMediumKinds::Internal ) )
{
}

void
CTransactionsKnownRequest::accept( common::CMedium< TrackerResponses > * _medium ) const
{
	_medium->add( this );
}

common::CMediumFilter< TrackerResponses > *
CTransactionsKnownRequest::getMediumFilter() const
{
	return common::CRequest< TrackerResponses >::m_mediumFilter;
}

CTransactionsAckRequest::CTransactionsAckRequest()
	: common::CRequest< TrackerResponses >( new CMediumClassFilter( common::CMediumKinds::Internal ) )
{
}

void
CTransactionsAckRequest::accept( common::CMedium< TrackerResponses > * _medium ) const
{
	_medium->add( this );
}

common::CMediumFilter< TrackerResponses > *
CTransactionsAckRequest::getMediumFilter() const
{
	return common::CRequest< TrackerResponses >::m_mediumFilter;
}

CTransactionsDoublespendRequest::CTransactionsDoublespendRequest()
	: common::CRequest< TrackerResponses >( new CMediumClassFilter( common::CMediumKinds::Internal ) )
{
}

void
CTransactionsDoublespendRequest::accept( common::CMedium< TrackerResponses > * _medium ) const
{
	_medium->add( this );
}

common::CMediumFilter< TrackerResponses > *
CTransactionsDoublespendRequest::getMediumFilter() const
{
	return common::CRequest< TrackerResponses >::m_mediumFilter;
}

CTransactionsNotOkRequest::CTransactionsNotOkRequest()
	: common::CRequest< TrackerResponses >( new CMediumClassFilter( common::CMediumKinds::Internal ) )
{
}

void
CTransactionsNotOkRequest::accept( common::CMedium< TrackerResponses > * _medium ) const
{
	_medium->add( this );
}

common::CMediumFilter< TrackerResponses > *
CTransactionsNotOkRequest::getMediumFilter() const
{
	return common::CRequest< TrackerResponses >::m_mediumFilter;
}

}
