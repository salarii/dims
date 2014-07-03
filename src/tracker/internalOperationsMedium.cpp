// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "internalOperationsMedium.h"
#include "addressToCoins.h"
#include "transactionRecordManager.h"
#include "validateTransactionsRequest.h"
#include "connectToTrackerRequest.h"
#include "common/manageNetwork.h"


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
	common::CAvailableCoins availableCoins;

	std::vector< uint256 > coinsHashes;

	std::vector< CCoins > coins;

	if (
		   !CAddressToCoinsViewCache::getInstance()->getCoins( _request->getKey(), coinsHashes )
		|| !CTransactionRecordManager::getInstance()->getCoins( coinsHashes, coins )
		)
	{
		m_trackerResponses.push_back( availableCoins );
		return;
	}


	std::transform( coinsHashes.begin(), coinsHashes.end(), coins.begin(),
		   std::inserter(availableCoins.m_availableCoins, availableCoins.m_availableCoins.end() ), std::make_pair<uint256,CCoins> );

	m_trackerResponses.push_back( availableCoins );
}

void
CInternalOperationsMedium::add( CConnectToTrackerRequest const *_request )
{
// in general  it is to slow to be  handled  this  way, but  as usual we can live with that for a while
	common::CSelfNode* node = common::CManageNetwork::getInstance()->connectNode( _request->getServiceAddress(), _request->getAddress().empty()? 0 : _request->getAddress().c_str() );

	m_trackerResponses.push_back( common::CConnectedNode( node ) );
}

void
CInternalOperationsMedium::add( common::CContinueReqest<TrackerResponses> const * _request )
{
	m_trackerResponses.push_back( common::CContinueResult( 0 ) );
}

void
CInternalOperationsMedium::add(CValidateTransactionsRequest const * _request )
{
	//this is  simplified to maximum, hence correct only in some cases
	bool valid = CTransactionRecordManager::getInstance()->validateTransactionBundle( _request->getTransactions() );
	m_trackerResponses.push_back( CValidationResult( valid ) );
}


bool
CInternalOperationsMedium::getResponse( std::vector< TrackerResponses > & _requestResponse ) const
{
	_requestResponse = m_trackerResponses;
	return true;
}

void
CInternalOperationsMedium::clearResponses()
{
	m_trackerResponses.clear();
}


}
