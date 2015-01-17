// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef AUTHENTICATION_PROVIDER_H
#define AUTHENTICATION_PROVIDER_H

#include "allocators.h"
#include "key.h"
#include "databaseManager.h"

class CCryptoKeyStore;

namespace common
{

class CIdentificationDB;

class CAuthenticationProvider
{
public:
	void setPassword( SecureString const & _strWalletPassphrase );

	bool hasKey( CKeyID const & _key ) const;

	void enableAccess() const;

	CPubKey getMyKey() const;

	bool sign( uint256 const &_hash, std::vector<unsigned char> & _vchSig ) const;

	bool verify( CKeyID const & _key, uint256 const & _hash, std::vector<unsigned char> const & _vchSig ) const;

	bool generateKeyPair( CKey & _priv, CPubKey & _pubKey );
	//bool addAddress( char * );

	void save();

	void load();

	void addPubKey( CPubKey const & _pubKey );

	bool getPubKey( CKeyID const & _key, CPubKey & _pubKey );
//	bool verify( CNode* _node, std::vector<unsigned char>& _vchSig ) const;

	static CAuthenticationProvider* getInstance();

	~CAuthenticationProvider(){};
private:
	bool isCrypted(){ return false; }

	CAuthenticationProvider();
private:
	static CAuthenticationProvider * ms_instance;

	CCryptoKeyStore * m_keyStore;
	
	static std::string const m_identificationFile;

	CIdentificationDB * m_identificatonDB;

	std::map< CKeyID, CPubKey > m_keys;

	CPubKey m_selfKey;
};

}

#endif // AUTHENTICATION_PROVIDER_H
