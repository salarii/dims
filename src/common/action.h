// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ACTION_H
#define ACTION_H

#include "setResponseVisitor.h"

namespace common
{

template < class _RequestResponses >
class CSetResponseVisitor;

template < class _RequestResponses > class CSetResponseVisitorBase;
template < class _RequestResponses > struct CRequest;

struct ActionStatus
{
	enum Enum
	{
         Unprepared
        , InProgress
        , Done
	};
};

template < class _RequestResponses >
class CAction
{
public:
	CAction(): m_actionStatus( ActionStatus::Unprepared ){};

	virtual void accept( CSetResponseVisitor< _RequestResponses > & _visitor );

	virtual CRequest< _RequestResponses >* execute() = 0;

	ActionStatus::Enum getState(){ return m_actionStatus; }

	virtual void reset(){ m_actionStatus = ActionStatus::Unprepared; }

	virtual ~CAction(){};
protected:
	ActionStatus::Enum m_actionStatus;
};

template < class _RequestResponses >
inline
void
CAction<_RequestResponses>::accept(CSetResponseVisitor<_RequestResponses> &_visitor )
{
}


}

#endif
