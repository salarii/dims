#include "setResponseVisitor.h"

namespace node
{

inline
CSetResponseVisitor::CSetResponseVisitor( RequestRespond const & _requestRespond )
	: m_requestRespond( _requestRespond )
{
}

void 
CSetResponseVisitor::visit( CSendTransaction & _sendTransaction )
{
	_sendTransaction.setTransactionStatus(boost::apply_visitor( GetTransactionStatus(), m_requestRespond ));
	_sendTransaction.setTransactionToken(boost::apply_visitor( GetToken(), m_requestRespond ));
}

void
CSetResponseVisitor::visit( CAction & _action )
{

}


}