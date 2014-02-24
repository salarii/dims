#ifndef TRANSACTION_RECORD_MANAGER_H
#define TRANSACTION_RECORD_MANAGER_H


class CTransactionRecordManager
{
public:
	CTransactionRecordManager();

private:
	void synchronize();
	void askForTokens();
};


#endif // TRANSACTION_RECORD_MANAGER_H