// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef WALLET_MANAGER_H
#define WALLET_MANAGER_H

#include "key.h"

#include "wallet.h"


namespace self
{

class CTransactionRecordManager;

class CWalletManager
{
public:
	CWalletManager();

	bool addAddress( std::string const & _privateKey );
	CPubKey generateAddress();
	void deleteAddress();

	void sendCoins( CKeyID const & _keyID, int64_t _value );
	bool checkCoinsPresent();

	void encryptWallet();
	void decriptWallet();

private:
	CWallet m_wallet;
	CTransactionRecordManager * m_transactionRecordManager;
};


}

#endif // WALLET_MANAGER_H
