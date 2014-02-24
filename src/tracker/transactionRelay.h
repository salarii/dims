#ifndef TRANSACTION_RELAY_H
#define TRANSACTION_RELAY_H

class CTransactionRelayPoint
{
public:
	CTransactionRelayPoint();
	void analyseTransactionBundle();
	void addTransactionToBundle();

	void passTransactionBundle();
private:
	void createTransactiopnBundle();

private:
	CMessageEncryptionLayer m_messageEncryptionLayer;
};


#endif // TRANSACTION_RELAY_H