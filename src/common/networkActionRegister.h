// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef NETWORK_ACTION_REGISTER_H
#define NETWORK_ACTION_REGISTER_H

#include "util.h"

namespace common
{

class CNetworkActionRegister
{
public:
	static CNetworkActionRegister* getInstance( );

	void registerServicedByAction( uint256 const & _actionKey );

	void unregisterServicedByAction( uint256 const & _actionKey );

	bool isServicedByAction( uint256 const & _actionKey ) const;
private:
	static CNetworkActionRegister * ms_instance;

	std::set< uint256 > m_actionsInProgress;
};

}

#endif // NETWORK_ACTION_REGISTER_H
