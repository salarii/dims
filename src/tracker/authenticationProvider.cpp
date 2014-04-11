// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "authenticationProvider.h"

namespace tracker
{
CAuthenticationProvider * CAuthenticationProvider::ms_instance = NULL;
	
CAuthenticationProvider*
CAuthenticationProvider::getInstance( )
{
	if ( !ms_instance )
	{
		ms_instance = new CAuthenticationProvider();
	};
	return ms_instance;
}

CAuthenticationProvider::CAuthenticationProvider()
{
	m_keyStore = new CCryptoKeyStore();
}

bool
CAuthenticationProvider::hasKeys( CKeyID const & _key ) const
{
	return m_keyStore->HaveKey( _key );
}

void
CAuthenticationProvider::setPassword( SecureString const & _strWalletPassphrase )
{
	if (isCrypted())
		return false;

	CKeyingMaterial vMasterKey;
	RandAddSeedPerfmon();

	vMasterKey.resize(WALLET_CRYPTO_KEY_SIZE);
	RAND_bytes(&vMasterKey[0], WALLET_CRYPTO_KEY_SIZE);

	CMasterKey kMasterKey;

	RandAddSeedPerfmon();
	kMasterKey.vchSalt.resize(WALLET_CRYPTO_SALT_SIZE);
	RAND_bytes(&kMasterKey.vchSalt[0], WALLET_CRYPTO_SALT_SIZE);

	CCrypter crypter;
	int64_t nStartTime = GetTimeMillis();
	crypter.SetKeyFromPassphrase(_strWalletPassphrase, kMasterKey.vchSalt, 25000, kMasterKey.nDerivationMethod);
	kMasterKey.nDeriveIterations = 2500000 / ((double)(GetTimeMillis() - nStartTime));

	nStartTime = GetTimeMillis();
	crypter.SetKeyFromPassphrase(_strWalletPassphrase, kMasterKey.vchSalt, kMasterKey.nDeriveIterations, kMasterKey.nDerivationMethod);
	kMasterKey.nDeriveIterations = (kMasterKey.nDeriveIterations + kMasterKey.nDeriveIterations * 100 / ((double)(GetTimeMillis() - nStartTime))) / 2;

	if (kMasterKey.nDeriveIterations < 25000)
		kMasterKey.nDeriveIterations = 25000;

	LogPrintf("Encrypting Wallet with an nDeriveIterations of %i\n", kMasterKey.nDeriveIterations);

	if (!crypter.SetKeyFromPassphrase(_strWalletPassphrase, kMasterKey.vchSalt, kMasterKey.nDeriveIterations, kMasterKey.nDerivationMethod))
		return false;
	if (!crypter.Encrypt(vMasterKey, kMasterKey.vchCryptedKey))
		return false;

	{
		LOCK(cs_wallet);
	//	mapMasterKeys[++nMasterKeyMaxID] = kMasterKey;

		m_keyStorageDataBase = new CWalletDB(m_authenticationProviderFile);
		if (!m_keyStorageDataBase->TxnBegin())
			return false;
		m_keyStorageDataBase->WriteMasterKey(nMasterKeyMaxID, kMasterKey);


		if (!m_keyStore->EncryptKeys(vMasterKey))
		{
			m_keyStorageDataBase->TxnAbort();
			exit(1); //We now probably have half of our keys encrypted in memory, and half not...die and let the user reload their unencrypted wallet.
		}

		if (!m_keyStorageDataBase->TxnCommit())
			exit(1); //We now have keys encrypted in memory, but no on disk...die to avoid confusion and let the user reload their unencrypted wallet.

		delete m_keyStorageDataBase;
		m_keyStorageDataBase = NULL;

	/*	Lock();
		Unlock(strWalletPassphrase);
		NewKeyPool();
		Lock();*/

		// Need to completely rewrite the wallet file; if we don't, bdb might keep
		// bits of the unencrypted private key in slack space in the database file.
		CDB::Rewrite(m_authenticationProviderFile);
	}

	return true;	
}

void 
CAuthenticationProvider::enableAccess() const
{
	
}

bool
CAuthenticationProvider::sign( CKeyID const & _key, uint256 const &_hash, std::vector<unsigned char> & _vchSig ) const
{
	CKey privKey;
	
	if ( !m_keyStore->GetKey( _key, privKey ) )
		return false;

	return privKey.Sign( _hash, _vchSig );
}

bool
CAuthenticationProvider::verify( CKeyID const & _key, uint256 const & _hash, std::vector<unsigned char> const & _vchSig ) const
{
	CKey privKey;

	std::map< CKeyID, CPubKey >::iterator iterator = m_pairsPubKeyStore.find( _key );
	if ( iterator == m_pairsPubKeyStore.end() )
		return false;

	return iterator->second.Verify( _hash, _vchSig );
}

CKeyID
CAuthenticationProvider::generateKeyPair()
{
	CKey priv;
	priv.MakeNewKey( false );
	
	m_keyStore.AddKey( priv );

	return priv->GetPubKey()->GetID();
}

CKeyID
CAuthenticationProvider::addAddress( char * _privPlain )
{
	CKey priv;
	
	priv.Set( _privPlain[0], _privPlain[size-1] );

	m_keyStore.AddKey(priv);

	return priv->GetPubKey()->GetID();
}


void
CAuthenticationProvider::save()
{

}

void
CAuthenticationProvider::load()
{

}



	bool verify( CNode* _node, std::vector<unsigned char>& _vchSig ) const;

}