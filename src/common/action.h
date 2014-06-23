// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ACTION_H
#define ACTION_H

//#include "setResponseVisitor.h"

namespace common
{

template < class _RequestResponses >
class CSetResponseVisitor;

template < class _RequestResponses > struct CRequest;
// this  is  obsolete
// but I need  some  wa to mark  that action  is  finished
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
	CAction( bool _autoDelete = true ): m_executed( false ), m_autoDelete( _autoDelete ){};

	virtual void accept( CSetResponseVisitor< _RequestResponses > & _visitor ) = 0;

	virtual CRequest< _RequestResponses >* execute() = 0;

	bool isExecuted(){ return m_executed; }

	void setExecuted(){ m_executed = true; }

	bool autoDelete(){ return m_autoDelete; }

	virtual void reset(){ m_executed = false; }

	virtual ~CAction(){};
protected:
	bool m_executed;

	bool const m_autoDelete;
};


}

#endif
