#include "validationManager.h"

namespace self
{

CValidationManager::CValidationManager()
{

}

void
CValidationManager::auditTransaction( CTransaction const & _tx )
{
	boost::lock_guard<boost::mutex> lock(buffMutex);

	m_transactionsCandidates.push_back( _tx );
}

void
CValidationManager::addBundleToSendQueue( std::vector< CTransaction > const & _bundle, TransactionStatus::Enum const _status )
{
	std::pair< std::vector< CTransaction >, TransactionStatus::Enum > bundle( _bundle, _status );

	boost::lock_guard<boost::mutex> lock(processedMutex);

	m_bundlePipe.push_back( bundle );
}

TransactionStatus::Enum
CValidationManager::pullCurrentTransactionStatus( CTransaction const & _tx )
{

}

void 
CValidationManager::workLoop()
{
	{
		boost::lock_guard<boost::mutex> lock(buffMutex);

		m_transactionRecordManager->handleTransactionBundle( m_transactionsCandidates );
		m_transactionsCandidates.clear();
	}

	{
		boost::lock_guard<boost::mutex> lock(processedMutex);

		std::list< std::pair< TransactionStatus::Enum, std::vector< CTransaction > > >::iterator iterator = m_bundlePipe.begin();

		while( iterator != m_bundlePipe.end() )
		{
			passTransactionBundleToNetwork( iterator->second, iterator->first);
			m_bundlePipe.erase( iterator );
		}
		
	}

	MilliSleep(500);
}

}