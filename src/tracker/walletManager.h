#ifndef WALLET_MANAGER_H
#define WALLET_MANAGER_H

class CWalletManager
{
public:
	CWalletManager();

	void addAddress( char * );
	void generateAddress();
	void deleteAddress();

	void sendCoins();
	bool checkCoinsPresent();

private:
	CAuthenticationProvider * m_authenticationProvider;

};


#endif // WALLET_MANAGER_H