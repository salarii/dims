// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "common/networkActionRegister.h"

namespace common
{

CNetworkActionRegister * CNetworkActionRegister::ms_instance = NULL;

CNetworkActionRegister*
CNetworkActionRegister::getInstance()
{
	if ( !ms_instance )
	{
		ms_instance = new CNetworkActionRegister();
	};
	return ms_instance;
}

bool
CNetworkActionRegister::isServicedByAction( uint256 const & _actionKey ) const
{
	std::set< uint256 >::const_iterator iterator = m_actionsInProgress.find( _actionKey );

	return iterator != m_actionsInProgress.end();
}

void
CNetworkActionRegister::unregisterServicedByAction( uint256 const & _actionKey )
{
	m_actionsInProgress.erase( _actionKey );
}


void
CNetworkActionRegister::registerServicedByAction( uint256 const & _actionKey )
{
	m_actionsInProgress.insert( _actionKey );
}


}

