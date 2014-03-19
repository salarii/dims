#include "actionHandler.h"

namespace node
{

inline
CVisitor::CVisitor( RequestRespond const & _requestRespond )
	: m_requestRespond( _requestRespond )
{
}
TransactionsStatus::Enum m_status;
uint256 m_token;
class GetTransactionStatus : public boost::static_visitor<int>
{
public:
	TransactionsStatus::Enum operator()(CTransactionStatus & _transactionStatus ) const
	{
		return i;
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
		return i;
	}

	uint256 operator()(boost::any & _any ) const
	{
		throw std::exception;
	}
};

void 
CVisitor::visit( CSendTransaction & _sendTransaction )
{

}

void
CVisitor::visit( CAction & _action )
{

}

void
accept( CVisitor & _visitor )
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
 
		BOOST_FOREACH(std::pair< CRequest*, CAction* > reqAction, m_reqToAction)
		{
			if ( m_requestHandler->isProcessed( reqAction.first ) )
			

		}
		sleep(1000);
	}
}


}