// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SET_RESPONSE_VISITOR_H
#define SET_RESPONSE_VISITOR_H

#include <boost/variant.hpp> 
#include "tracker/configureTrackerActionHandler.h"

namespace tracker
{
class CGetBalanceAction;
}

namespace common
{

class CSendTransactionAction;

template < class _RequestResponses > class CAction;

class CConnectAction;

template < class _RequestResponses >
class CSetResponseVisitor
{
public:
	CSetResponseVisitor( std::list< _RequestResponses > const & _requestRespond );
	virtual void visit( CAction< _RequestResponses > & _action );
private:

	std::list< _RequestResponses > m_requestRespond;
};

template < class _RequestResponses >
inline
CSetResponseVisitor< _RequestResponses >::CSetResponseVisitor( std::list< _RequestResponses > const & _requestRespond )
: m_requestRespond( _requestRespond )
{
}


template < class _RequestResponses >
inline
void
CSetResponseVisitor< _RequestResponses >::visit( CAction< _RequestResponses > & _action )
{
}

/*
template<>
class CSetResponseVisitor< tracker::TrackerResponses >
{
public:
	CSetResponseVisitor( tracker::TrackerResponses const & _trackerResponses ){};

	virtual void visit( common::CAction< tracker::TrackerResponses > & _action ){};

	virtual void visit( tracker::CGetBalanceAction & _action ){};

};

*/

}

#endif
