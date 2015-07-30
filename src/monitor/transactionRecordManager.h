// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef TRANSACTION_RECORD_MANAGER_H
#define TRANSACTION_RECORD_MANAGER_H

#include "common/originAddressScanner.h"

//don't know yet if this  should be  the same as in tracker
//
namespace monitor
{

class CTransactionRecordManager : public common::CStorageBase
{
public:
	static CTransactionRecordManager* getInstance( );

	void addCoinbaseTransaction( CTransaction const & _tx, CKeyID const & _keyId  );

	void addTransactionToStorage( CTransaction const & _tx );

private:
	CTransactionRecordManager();
private:
	static CTransactionRecordManager * ms_instance;
};

}
#endif // TRANSACTION_RECORD_MANAGER_H
