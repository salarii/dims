// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <openssl/rand.h>

#include "authenticationProvider.h"
#include "script.h"
#include "crypter.h"
#include "util.h"
#include "db.h"
#include "serialize.h"

namespace common
{

std::string const CAuthenticationProvider::m_identificationFile = "identification";

CAuthenticationProvider * CAuthenticationProvider::ms_instance = NULL;

CAuthenticationProvider*
CAuthenticationProvider::getInstance()
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

	m_identificatonDB = new CIdentificationDB( m_identificationFile, "rc+" );

	m_identificatonDB->loadIdentificationDatabase( m_keys, m_selfKey, m_keyStore );

	if ( !m_selfKey.IsValid() )
	{
		CKey priv;
		CPubKey pubKey;

		generateKeyPair( priv, pubKey );

		m_identificatonDB->writeKeySelf( pubKey, priv.GetPrivKey() );

		m_selfKey = pubKey;

		m_keyStore->AddKeyPubKey(priv, pubKey);

	}

}

CPubKey
CAuthenticationProvider::getMyKey() const
{
	return m_selfKey;
}

/*
bool
CAuthenticationProvider::hasKeys( CKeyID const & _key ) const
{
	return m_keyStore->HaveKey( _key );
}
*/
void
CAuthenticationProvider::setPassword( SecureString const & _strWalletPassphrase )
{
	if (isCrypted())
		return;

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
		return;
	if (!crypter.Encrypt(vMasterKey, kMasterKey.vchCryptedKey))
		return;

	{
	//	mapMasterKeys[++nMasterKeyMaxID] = kMasterKey;
/*
		CWalletDB * m_keyStorageDataBase = new CWalletDB(m_authenticationProviderFile);
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

		Lock();
		Unlock(strWalletPassphrase);
		NewKeyPool();
		Lock();

		// Need to completely rewrite the wallet file; if we don't, bdb might keep
		// bits of the unencrypted private key in slack space in the database file.
		CDB::Rewrite(m_authenticationProviderFile);*/
	}

	return;
}

void 
CAuthenticationProvider::enableAccess() const
{
	
}

bool
CAuthenticationProvider::sign( uint256 const &_hash, std::vector<unsigned char> & _vchSig ) const
{
	if ( !m_selfKey.IsValid() )
		return false;

	CKey privKey;

	if ( !m_keyStore->GetKey( m_selfKey.GetID(), privKey ) )
		return false;

	return privKey.Sign( _hash, _vchSig );
}

bool
CAuthenticationProvider::verify( CKeyID const & _key, uint256 const & _hash, std::vector<unsigned char> const & _vchSig ) const
{
	std::map< CKeyID, CPubKey >::const_iterator iterator = m_keys.find( _key );
	if ( iterator == m_keys.end() )
		return false;

	return iterator->second.Verify( _hash, _vchSig );
}

bool
CAuthenticationProvider::generateKeyPair( CKey & _priv, CPubKey & _pubKey )
{
	_priv.MakeNewKey( false );

	_pubKey = _priv.GetPubKey();

	return true;
}


void
CAuthenticationProvider::addPubKey( CPubKey const & _pubKey )
{
	m_keys.insert( std::make_pair( _pubKey.GetID(), _pubKey ) );
}

bool
CAuthenticationProvider::getPubKey( CKeyID const & _key, CPubKey & _pubKey )
{
	std::map< CKeyID, CPubKey >::const_iterator iterator = m_keys.find( _key );

	if ( iterator == m_keys.end() )
		return false;

	_pubKey = iterator->second;
	return true;
}


void
CAuthenticationProvider::save()
{

}

void
CAuthenticationProvider::load()
{

}


}
