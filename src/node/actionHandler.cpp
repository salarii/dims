#include "actionHandler.h"

namespace node
{

unsigned int const CActionHandle::m_sleepTime = 2;

TransactionsStatus::Enum m_status;
uint256 m_token;
class GetTransactionStatus : public boost::static_visitor<int>
{
public:
	TransactionsStatus::Enum operator()(CTransactionStatus & _transactionStatus ) const
	{
		return _transactionStatus.m_status;
	}

	TransactionsStatus::Enum operator()(boost::any & _any ) const
	{
		throw std::exception;
	}
};

class GetToken : public boost::static_visitor<int>
{
public:
	uint256 operator()(CTransactionStatus & _transactionStatus ) const
	{
		return _transactionStatus.m_token;
	}

	uint256 operator()(boost::any & _any ) const
	{
		throw std::exception;
	}
};

void
accept( CSetResponseVisitor & _visitor )
{
	_visitor( *this );
}

void 
CActionHandle::run()
{
	while(1)
	{
		{
			QMutexLocker lock( &m_mutex );
			BOOST_FOREACH(CAction* action, m_actions)
			{
				CRequest* request = action->execute();

				if ( request )
					m_reqToAction.insert( std::make_pair( request, action ) );
			}

			m_actions.clear();

		}
 
		std::list<>
		BOOST_FOREACH(std::pair< CRequest*, CAction* > reqAction, m_reqToAction)
		{
			if ( m_requestHandler->isProcessed( reqAction.first ) )
			{
				
				CSetResponseVisitor visitor( m_requestHandler->getRespond( reqAction.first ) );
				reqAction.second.accept( visitor );
				
				m_actions.push_back( reqAction.second );
			}
		}
		QThread::sleep ( m_sleepTime );
	}
}


}