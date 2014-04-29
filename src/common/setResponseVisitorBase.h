// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SET_RESPONSE_VISITOR_BASE_H
#define SET_RESPONSE_VISITOR_BASE_H

namespace common
{

template < class _RequestResponses > class CAction;

template < class _RequestResponses >
class CSetResponseVisitorBase
{
public:
	virtual void visit( CAction< _RequestResponses > & _action );
};

template < class _RequestResponses >
void
CSetResponseVisitorBase< _RequestResponses >::visit( CAction< _RequestResponses > & _action )
{
}

}

#endif // SET_RESPONSE_VISITOR_BASE_H
