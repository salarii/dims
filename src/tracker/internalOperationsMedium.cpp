// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "internalOperationsMedium.h"
#include "addressToCoins.h"
#include "transactionRecordManager.h"
#include "common/manageNetwork.h"

#include "tracker/requests.h"

#include <algorithm>

namespace tracker
{

CInternalOperationsMedium * CInternalOperationsMedium::ms_instance = NULL;

CInternalOperationsMedium*
CInternalOperationsMedium::getInstance()
{
	if ( !ms_instance )
	{
		ms_instance = new CInternalOperationsMedium();
	};
	return ms_instance;
}

CInternalOperationsMedium::CInternalOperationsMedium()
{
}

bool
CInternalOperationsMedium::serviced() const
{
	return true;
}

void
CInternalOperationsMedium::add( CGetBalanceRequest const *_request )
{
	common::CAvailableCoinsData availableCoins;

	std::vector< uint256 > coinsHashes;

	std::vector< CCoins > coins;

	if (
		   ! CAddressToCoinsViewCache::getInstance()->getCoins( _request->getKey(), coinsHashes )
		|| !CTransactionRecordManager::getInstance()->getCoins( coinsHashes, coins )
		)
	{
		m_trackerResponses.insert( std::make_pair( (common::CRequest< common::CTrackerTypes >*)_request, availableCoins ) );
		return;
	}


	std::transform( coinsHashes.begin(), coinsHashes.end(), coins.begin(),
		   std::inserter(availableCoins.m_availableCoins, availableCoins.m_availableCoins.end() ), std::make_pair<uint256,CCoins> );

	m_trackerResponses.insert( std::make_pair( (common::CRequest< common::CTrackerTypes >*)_request, availableCoins ) );
}

void
CInternalOperationsMedium::add( CConnectToTrackerRequest const *_request )
{
// in general  it is to slow to be  handled  this  way, but  as usual we can live with that for a while
	common::CSelfNode* node = common::CManageNetwork::getInstance()->connectNode( _request->getServiceAddress(), _request->getAddress().empty()? 0 : _request->getAddress().c_str() );

	m_trackerResponses.insert( std::make_pair( (common::CRequest< common::CTrackerTypes >*)_request, common::CConnectedNode( node ) ) );
}

void
CInternalOperationsMedium::add(CValidateTransactionsRequest const * _request )
{
	std::vector< unsigned int > invalidTransactions;
	CTransactionRecordManager::getInstance()->validateTransactionBundle( _request->getTransactions(), invalidTransactions );

	m_trackerResponses.insert( std::make_pair( (common::CRequest< common::CTrackerTypes >*)_request, CValidationResult( invalidTransactions ) ) );
}


bool
CInternalOperationsMedium::getResponseAndClear( std::multimap< common::CRequest< common::CTrackerTypes >const*, TrackerResponses > & _requestResponse )
{
	_requestResponse = m_trackerResponses;

	clearResponses();
	return true;
}

void
CInternalOperationsMedium::clearResponses()
{
	m_trackerResponses.clear();
}


}
