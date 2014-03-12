#include "chargeRegister.h"

namespace monitor
{

struct CChargeMessage
{
};

CChargeRegister::CChargeRegister()
{

}

void
CChargeRegister::getChargeAmount()
{
}

void
CChargeRegister::checkCharge()
{

}

void
CChargeRegister::threatDelisting( uint256 const & _trackerHash )
{
	CChargeMessage
	checkCharge()
	m_messagingHandler->sendMessage();
}

void
CChargeRegister::registerPubKey( std::pair _pubKey )
{
	m_registeredPubKeys.insert( _pubKey );
}

void
CChargeRegister::loop()
{
	while(1)
	{
		std::list< uint256 >::iterator iterator = m_enlisted.begin();

		while( iterator != m_enlisted.end() )
		{
			if ( getNextPayTime( *iterator ) - m_warningTime < GetTime() )
				threatDelisting( *iterator );


		}

		sleep( 6 hours );
	}
}

}