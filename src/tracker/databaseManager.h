#ifndef DATABASE_MANAGER_H
#define DATABASE_MANAGER_H

namespace Self
{


typedef boost::variant< GetBitcoinHeaderTip > DatabaseOperationType;
DatabaseOperationType::types::
class CDatabaseManager
{
	//mutex nie  mo¿e  byæ  przerwane
public:
	CDatabaseManager();
	
	bool getLastBlockHash( uint256 & _hash );

	bool SetLastBlockHash( uint256 const &hash );

	bool getBalanceForHash( uint256 const &_hash );

	//serious  data  storage  from  this  recovery

private:
	CBlockTreeDB m_blockTreeDB;
};

}

#endif // DATABASE_MANAGER_H
