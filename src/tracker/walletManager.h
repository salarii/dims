// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef WALLET_MANAGER_H
#define WALLET_MANAGER_H

namespace self
{

class CWalletManager
{
public:
	CWalletManager();

	bool addAddress( SecureString _privateKey );
	CPubKey generateAddress();
	void deleteAddress();

	void sendCoins();
	bool checkCoinsPresent();

	void encryptWallet();
	void decriptWallet();

private:
	CWallet m_wallet;
	CValidationManager * m_validationManager;
};


}

#endif // WALLET_MANAGER_H