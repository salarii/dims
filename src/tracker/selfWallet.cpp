// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "wallet.h"

#include "tracker/selfWallet.h"

extern CWallet* pwalletMain;

namespace tracker
{

CSelfWallet * CSelfWallet::ms_instance = NULL;

CSelfWallet*
CSelfWallet::getInstance( )
{
	if ( !ms_instance )
	{
		ms_instance = new CSelfWallet();
	};
	return ms_instance;
}





}
