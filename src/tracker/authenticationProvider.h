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