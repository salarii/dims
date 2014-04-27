// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


#ifndef SET_RESPONSE_VISITOR_H
#define SET_RESPONSE_VISITOR_H

#include <boost/variant.hpp> 

#include "requestHandler.h"


/*
#include <boost/mpl/list.hpp>
#include <boost/mpl/less.hpp>
#include <boost/mpl/size.hpp>
#include <boost/mpl/at.hpp>
#include <boost/mpl/if.hpp>

#define VisitorParam( num )  if_< less< int_< num >, size< ParametersList >::type >::type, at< ParametersList ,if_< less< int_< num >, size< ParametersList >::type >::type, int_< num >, int_< 0 > >::type >::type ,at< DummyList ,int_< num > >::type  >::type

using namespace boost::mpl;

struct D1{};struct  D2{};struct  D3{};struct D4{};struct  D5{};struct  D6{};
typedef list< D1,D2,D3,D4,D5,D6 >  DummyList;

typedef list< int, float >  ParametersList;

 */
namespace common
{

class CSendTransactionAction;

class CAction;

class CConnectAction;

class CSetResponseVisitor
{
public:
	CSetResponseVisitor( std::list< RequestResponse > const & _requestRespond );

	void visit( CSendTransactionAction & _action );

	void visit( CConnectAction & _action );

	void visit( CAction & _action );
private:

	std::list< RequestResponse > m_requestRespond;
};

inline
CSetResponseVisitor::CSetResponseVisitor( std::list< RequestResponse > const & _requestRespond )
: m_requestRespond( _requestRespond )
{
}


}

#endif
