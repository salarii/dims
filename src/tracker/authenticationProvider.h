// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
/*

FILE*
OpenHeadFile(bool fReadOnly)
{
    return OpenDiskFile(CDiskBlockPos(), "head", fReadOnly);
}


CAutoFile file(OpenHeadFile(false), SER_DISK, CLIENT_VERSION);
file << header;


fflush(file);

FileCommit(file);




CAutoFile file(OpenHeadFile(true), SER_DISK, CLIENT_VERSION);
file >> header;



std::map< long long, uint256 > requestedMerkle;

hash * store;

if ( hash == requestedMerkle.begin().second )
{
	store = requestedMerkle.begin();
	requestedMerkle.erase(requestedMerkle.begin())
}
else
{
	achievedMerkle.insert( height , hash )
}




std::map< long long, uint256 >::iterator it = achievedMerkle.begin();

while( it != achievedMerkle.end() )
{
	if ( it->second == requestedMerkle.begin().second )
	{
		store = requestedMerkle.begin();
		it++;
		achievedMerkle.erase(achievedMerkle.begin());
		requestedMerkle.erase(requestedMerkle.begin())
	}
	else
		break;

}
*/

#ifndef AUTHENTICATION_PROVIDER_H
#define AUTHENTICATION_PROVIDER_H

namespace Self
{

class CAuthenticationProvider
{
public:
	CAuthenticationProvider();

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
private:
	bool isCrypted();
private:
	CCryptoKeyStore * m_keyStore;
	
	std::map< CKeyID, CPubKey > m_pairsPubKeyStore;

	CWalletDB *m_keyStorageDataBase;
	
	std::string m_authenticationProviderFile;
};

}

#endif // AUTHENTICATION_PROVIDER_H
