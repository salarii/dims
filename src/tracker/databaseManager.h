// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef DATABASE_MANAGER_H
#define DATABASE_MANAGER_H

namespace Self
{


typedef boost::variant< GetBitcoinHeaderTip > DatabaseOperationType;
DatabaseOperationType::types::
class CDatabaseManager
{
	//mutex nie  mo�e  by�  przerwane
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
