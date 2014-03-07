// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

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