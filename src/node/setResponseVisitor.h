// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


#ifndef SET_RESPONSE_VISITOR_H
#define SET_RESPONSE_VISITOR_H

#include <boost/variant.hpp> 

#include "requestHandler.h"

namespace node
{

class CSendTransaction;

class CAction;

class CSetResponseVisitor
{
public:
	CSetResponseVisitor( RequestRespond const & _requestRespond );

	void visit( CSendTransaction & _sendTransaction );

	void visit( CAction & _action );
private:

	RequestRespond m_requestRespond;
};


}

#endif