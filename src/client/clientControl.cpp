// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/state.hpp>
#include <boost/statechart/transition.hpp>

#include "common/actionHandler.h"
#include "common/periodicActionExecutor.h"

#include "sendBalanceInfoAction.h"
#include "connectAction.h"
#include "clientEvents.h"
#include "addresstablemodel.h"
#include "clientControl.h"

#include "ui_interface.h"

#include "applicationServer.h"

#include <QMessageBox>

namespace client
{
CClientControl * CClientControl::ms_instance = 0;

unsigned int const CheckBalancePeriod = 30000;

struct CClientConnected;

struct CUninitiatedClient : boost::statechart::simple_state< CUninitiatedClient, CClientControl >
{
	CUninitiatedClient()
	{
		CLocalServer::getInstance();
		common::CActionHandler::getInstance()->executeAction( new CConnectAction() );
	}

	typedef boost::mpl::list<
	boost::statechart::transition< CNetworkDiscoveredEvent, CClientConnected >
	> reactions;
};

struct CClientConnected : boost::statechart::state< CClientConnected, CClientControl >
{
	CClientConnected( my_context ctx ) : my_base( ctx )
	{
		// set to gui that it is connected to network
		CNetworkDiscoveredEvent const* discoveredEvent
				= dynamic_cast< CNetworkDiscoveredEvent const* >( simple_state::triggering_event() );

		uiInterface.NotifyNumConnectionsChanged( discoveredEvent->m_trackers, discoveredEvent->m_monitors );

		common::CPeriodicActionExecutor::getInstance()->addAction( new CSendBalanceInfoAction( false ), CheckBalancePeriod );
	}
};


CClientControl*
CClientControl::getInstance()
{
	if ( !ms_instance )
	{
		ms_instance = new CClientControl();
	};
	return ms_instance;
}

void
CClientControl::setAddressTableModel( AddressTableModel * _addressTableModel )
{
	m_addressTableModel = _addressTableModel;
}

std::vector< std::string >
CClientControl::getAvailableAddresses() const
{
	if ( !m_addressTableModel )
		return std::vector< std::string >();
	return m_addressTableModel->getAddresses();
}

void
CClientControl::updateTotalBalance( int64_t _totalBalance )
{
	m_clientSignals.m_updateTotalBalance( _totalBalance );
}

void
CClientControl::addTransactionToModel( CTransaction const & _transaction )
{
	m_clientSignals.m_putTransactionIntoModel( _transaction );
}

bool
CClientControl::determineFeeAndTracker( unsigned int _transactionAmount, common::CTrackerStats & _tracker, unsigned int & _fee )
{
	return CTrackerLocalRanking::getInstance()->determineTracker( _transactionAmount, _tracker, _fee );
}

bool
CClientControl::createTransaction( std::vector< std::pair< CKeyID, int64_t > > const & _outputs, std::vector< CSpendCoins > const & _coinsToUse, common::CTrackerStats const & _trackerStats,CWalletTx& _wtxNew, std::string& _strFailReason )
{
	return m_clientSignals.m_createTransaction( _outputs, _coinsToUse, _trackerStats, _wtxNew, _strFailReason );
}

bool
CClientControl::executePaymentMessageBox()
{
	return m_clientSignals.m_messageboxPaymentRequest().get_value_or(0) == QMessageBox::Ok ? true : false;
}

void
CClientControl::transactionAddmited( uint256 const & _hash, CTransaction const & _transaction )
{
	m_clientSignals.m_transactionAddmited( _hash, _transaction );
}

CClientSignals &
CClientControl::acquireClientSignals()
{
	return m_clientSignals;
}

CClientControl::CClientControl()
	: m_addressTableModel( 0 )
{
	initiate();
}

}
