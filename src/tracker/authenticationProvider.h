
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


std::map< long long, uint256 > achievedMerkle;

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


#ifndef AUTHENTICATION_PROVIDER_H
#define AUTHENTICATION_PROVIDER_H

namespace Self
{

class CAuthenticationProvider
{
public:
	CAuthenticationProvider();

	bool hasKeys() const;
	void enableAccess() const;

	bool generateKeyPair();
	void addAddress( char * );

	void save();
	void load();
private:
	CKeyStore * m_keyStore;
};

void addAddress( char * _privPlain )
{
	CKey priv;
	priv.Set( _privPlain[0], _privPlain[size-1] );
	m_keyStore.AddKey(priv);
}

bool
generateKeyPair()
{
	CKey priv;
	priv.MakeNewKey( false );
	m_keyStore.AddKey(priv);

//	CPrivKey GetPrivKey() const;
//	CPubKey GetPubKey() const;
}

}

#endif // AUTHENTICATION_PROVIDER_H
