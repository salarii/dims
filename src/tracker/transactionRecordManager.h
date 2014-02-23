#define TRANSACTION_RECORD_MANAGER_H
#endif TRANSACTION_RECORD_MANAGER_H

class CTransactionRecordManager
{
public:
	CTransactionRecordManager();

private:
	void synchronize();
	void askForTokens();
};


#endif // TRANSACTION_RECORD_MANAGER_H