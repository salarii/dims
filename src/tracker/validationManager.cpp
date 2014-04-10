#include "validationManager.h"

namespace tracker
{

CValidationManager::CValidationManager()
{
}

void
CValidationManager::serviceTransaction( CTransaction const & _tx )
{
	boost::lock_guard<boost::mutex> lock(buffMutex);

	m_transactionsCandidates.push_back( _tx );
}

void
CValidationManager::addBundleToSendQueue( std::vector< CTransaction > const & _bundle )
{
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
		boost::lock_guard<boost::mutex> lock(validateTransLock);
		std::map< uint256, std::vector< CTransaction > > 
		BOOST_FOREACH( std::pair< uint256, std::vector< CTransaction > & bundle, m_relayedTransactions )
		{
			if ( !m_validated.find( bundle->first ) )
			{
				if ( m_transactionRecordManager->validateTransactionBundle( bundle->second ) );
				{
					m_validated.insert( bundle->first );
				}
			}
		}
	}

	if ( m_relayedTransactions.find( uint256() ) == m_relayedTransactions.end() )
	{
		boost::lock_guard<boost::mutex> lock(buffMutex);
		m_relayedTransactions.insert( std::make_pair( uint256(), m_transactionsCandidates ) );
		m_transactionsCandidates.clear();
	}

	BOOST_FOREACH(uint256 & hash, m_validated )
	{
		// here plece  code  rlated  to  propagation of bundles  within  network as  well as  updating transaction  register with correct  transactions
		/*
		boost::lock_guard<boost::mutex> lock(processedMutex);

		std::list< std::pair< TransactionStatus::Enum, std::vector< CTransaction > > >::iterator iterator = m_bundlePipe.begin();

		while( iterator != m_bundlePipe.end() )
		{
			passTransactionBundleToNetwork( iterator->second, iterator->first);
			m_bundlePipe.erase( iterator );
		}
		*/
	}
}

}