// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


#ifndef SET_RESPONSE_VISITOR_H
#define SET_RESPONSE_VISITOR_H

#include <boost/variant.hpp> 

#include "requestHandler.h"

namespace common
{

class CSendTransactionAction;

class CAction;

class CConnectAction;

class CSetResponseVisitorBase
{
public:
	virtual void visit( CAction & _action );
};

void
CSetResponseVisitorBase::visit( CAction & _action )
{
}


template < class _RequestResponses >
class CSetResponseVisitor : public CSetResponseVisitorBase
{
public:
	CSetResponseVisitor( std::list< _RequestResponses > const & _requestRespond );
private:

	std::list< _RequestResponses > m_requestRespond;
};

template < class _RequestResponses >
inline
CSetResponseVisitor< _RequestResponses >::CSetResponseVisitor( std::list< _RequestResponses > const & _requestRespond )
: m_requestRespond( _requestRespond )
{
}


}

#endif
