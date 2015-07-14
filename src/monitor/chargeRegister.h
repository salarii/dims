// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef CHARGE_REGISTER_H
#define CHARGE_REGISTER_H

#include "uint256.h"

namespace monitor
{

class CChargeRegister
{
public:
	CChargeRegister();

	void getChargeAmount();

	void checkCharge();
	
	void threatDelisting( uint256 const & _trackerHash );
	
	void loop();

	void registerPubKey( std::pair _pubKey );
private:
	int64_t getNextPayTime( uint256 const & _trackerHash );
private:
	std::list< uint256 > m_enlisted;

	std::map < uint256, CPubKey > m_registeredPubKeys;

	CMessagingHandler * m_messagingHandler;

	int64_t  m_warningTime;
};

}

#endif
