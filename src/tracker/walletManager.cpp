// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "walletManager.h"

#include "base58.h"

namespace self
{

CWalletManager::CWalletManager()
{

}

/*
convert  private string to real private
*/
bool
CWalletManager::addAddress( std::string const & _privateKey )
{
	CBitcoinSecret secret;
	secret.SetString(_privateKey);

	m_wallet.AddKeyPubKey(secret.GetKey(), secret.GetKey().GetPubKey());
}

CPubKey
CWalletManager::generateAddress()
{
	return m_wallet.GenerateNewKey();
}

void
CWalletManager::deleteAddress()
{
}


void
CWalletManager::sendCoins( CKeyID const & _keyID, int64_t _value )
{
	CWalletTx wtxNew;
	m_wallet.SendMoneyToDestination(CTxDestination( _keyID ), _value, wtxNew );
	
//	m_validationManager->auditTransaction( wtxNew );
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
