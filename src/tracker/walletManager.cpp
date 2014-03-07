// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "walletsManager.h"

namespace self
{

CWalletManager::CWalletManager()
{

}

/*
convert  private string to real private
*/
bool
CWalletManager::addAddress( SecureString _privateKey )
{
	CKey privKey;
	privKey->Set(_privateKey.begin(), _privateKey.end(), false);

	m_wallet->AddKeyPubKey(privKey, privKey.GetPubKey());
}

CPubKey
CWalletManager::generateAddress()
{
	return m_wallet->GenerateNewKey();
}

void
CWalletManager::deleteAddress()
{
}


void
CWalletManager::sendCoins( CKeyID const & _keyID, int64_t _value )
{
	CWalletTx& wtxNew;
	m_wallet->SendMoneyToDestination(CTxDestination( _keyID ), _value, CWalletTx& wtxNew );
	
	m_validationManager->auditTransaction( wtxNew );
}

bool
CWalletManager::checkCoinsPresent()
{

}

void
CWalletManager::encryptWallet()
{

}

void
CWalletManager::decriptWallet()
{

}


}
