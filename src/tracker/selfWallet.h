// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SELF_WALLET_H
#define SELF_WALLET_H

#include "util.h"

class CWallet;

namespace tracker
{

class CSelfWallet
{
public:
	void createTransaction( int64_t _amount );

	static CSelfWallet* getInstance( );
private:
	static CSelfWallet * ms_instance;

	CSelfWallet();

	CWallet* m_wallet;
};

}

#endif // SELF_WALLET_H
