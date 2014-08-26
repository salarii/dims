// Copyright (c) 2014 Dims dev-team
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

namespace client
{
CClientControl * CClientControl::ms_instance = 0;

struct CClientConnected;

struct CUninitiatedClient : boost::statechart::simple_state< CUninitiatedClient, CClientControl >
{
	CUninitiatedClient()
	{
		common::CActionHandler< client::NodeResponses >::getInstance()->executeAction( new CConnectAction() );
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

		common::CPeriodicActionExecutor< client::NodeResponses >::getInstance()->addAction( new CSendBalanceInfoAction( false ), 6000 );
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
	return m_addressTableModel->getAddresses();
}

CClientControl::CClientControl()
{
	initiate();
}

}
