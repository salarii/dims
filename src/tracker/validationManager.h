// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef VALIDATION_MANAGER_H
#define VALIDATION_MANAGER_H

namespace self
{
struct TransactionStatus
{
	enum Enum
	{
		 Invalid = 0
		, Unconfirmed
		, Ack
		// , DoubleSpend
		, Confirmed
		, Investigated
	};
};

class CValidationManager
{
public:
	CValidationManager();
	
	void auditTransaction( CTransaction const & _tx );

	TransactionStatus::Enum pullCurrentTransactionStatus( CTransaction const & _tx );
private:
	void workLoop();

	void  passTransactionBundleToNetwork( std::vector< CTransaction > _transactionBundle , TransactionStatus::Enum _status );

	void addBundleToSendQueue( std::vector< CTransaction > _bundle );

	CTransactionRecordManager m_transactionRecordManager;
private:
	boost::mutex buffMutex;
	std::vector< CTransaction > m_transactionsCandidates;

	boost::mutex processedMutex;

	std::list< std::pair< TransactionStatus::Enum, std::vector< CTransaction > > > m_bundlePipe;

/*
pyramid
	char dirtyTable
	CTransaction 

	*/
};


}
/*
char [ n ]

mutex  with 

for nodes dirty table 
and  pointers  to it 


*/
#endif
