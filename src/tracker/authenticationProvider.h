// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef AUTHENTICATION_PROVIDER_H
#define AUTHENTICATION_PROVIDER_H

namespace tracker
{

class CAuthenticationProvider
{
public:
	void setPassword( SecureString const & _strWalletPassphrase );

	bool hasKey( CKeyID const & _key ) const;

	void enableAccess() const;

	uint256 sign( std::vector<unsigned char>& _vchSig ) const;

	void verify( ) const;

	bool generateKeyPair();
	void addAddress( char * );

	void save();
	void load();

	bool verify( CNode* _node, std::vector<unsigned char>& _vchSig ) const;

	static CAuthenticationProvider* getInstance( );
private:
	bool isCrypted();

	CAuthenticationProvider();
private:
	static CAuthenticationProvider * ms_instance;

	CCryptoKeyStore * m_keyStore;
	
	std::map< CKeyID, CPubKey > m_pairsPubKeyStore;

	CWalletDB *m_keyStorageDataBase;
	
	std::string m_authenticationProviderFile;
};

}

#endif // AUTHENTICATION_PROVIDER_H
