// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "common/segmentFileStorage.h"
#include "common/supportTransactionsDatabase.h"

#include "monitor/transactionRecordManager.h"

namespace monitor
{

CTransactionRecordManager * CTransactionRecordManager::ms_instance = NULL;

CTransactionRecordManager::CTransactionRecordManager()
{}

CTransactionRecordManager*
CTransactionRecordManager::getInstance( )
{
	if ( !ms_instance )
	{
		ms_instance = new CTransactionRecordManager();
	};
	return ms_instance;
}

//for  now  empty
void
CTransactionRecordManager::addCoinbaseTransaction( CTransaction const & _tx, CKeyID const & _keyId  )
{}

void
CTransactionRecordManager::addTransactionToStorage( CTransaction const & _tx )
{
	CTransaction tx( _tx );
	tx.m_location = common::CSegmentFileStorage::getInstance()->assignPosition( _tx );

	common::CSupportTransactionsDatabase::getInstance()->setTransactionLocation( _tx.GetHash(), tx.m_location );
	common::CSegmentFileStorage::getInstance()->includeTransaction( tx, GetTime() );

	common::CSupportTransactionsDatabase::getInstance()->flush();
}

bool
CTransactionRecordManager::addTransactionsToStorage( std::vector< CTransaction > const & _transaction )
{
	BOOST_FOREACH( CTransaction const & transaction, _transaction )
	{
		CTransaction tx( transaction );
		tx.m_location = common::CSegmentFileStorage::getInstance()->assignPosition( tx );

		common::CSupportTransactionsDatabase::getInstance()->setTransactionLocation( tx.GetHash(), tx.m_location );
		common::CSegmentFileStorage::getInstance()->includeTransaction( tx, GetTime() );
	}
	common::CSupportTransactionsDatabase::getInstance()->flush();
	return true;
}

}

