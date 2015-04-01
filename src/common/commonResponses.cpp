// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "commonResponses.h"

namespace common
{

CMainRequestType::Enum const CAvailableCoins::m_requestType = CMainRequestType::BalanceInfoReq;

CAvailableCoins::CAvailableCoins( std::map< uint256, CCoins > const & _availableCoins, uint256 const & _hash )
	: m_hash(_hash)
	, m_availableCoins( _availableCoins )
{
}

CPayApplicationData::CPayApplicationData(
		CTransaction const & _trasaction
		, std::vector<unsigned char> const & _transactionStatusSignature
		, CPubKey const & _servicingTracker
		, common::CMonitorData const & _monitorData
		, CPubKey const & _servicingMonitor
		)
	: m_trasaction( _trasaction )
	, m_transactionStatusSignature( _transactionStatusSignature )
	, m_servicingTracker( _servicingTracker )
	, m_monitorData( _monitorData )
	, m_servicingMonitor( _servicingMonitor )
{}

CAvailableCoins::CAvailableCoins()
{

}

}
