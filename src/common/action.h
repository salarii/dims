// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ACTION_H
#define ACTION_H

namespace common
{

class CSetResponseVisitorBase;
struct CRequest;

struct ActionStatus
{
	enum Enum
	{
         Unprepared
        , InProgress
        , Done
	};
};

class CAction
{
public:
	CAction(): m_actionStatus( ActionStatus::Unprepared ){};

	virtual void accept( CSetResponseVisitorBase & _visitor );

	virtual CRequest* execute() = 0;

	ActionStatus::Enum getState(){ return m_actionStatus; }

	virtual void reset(){ m_actionStatus = ActionStatus::Unprepared; }
protected:
	ActionStatus::Enum m_actionStatus;
};



}

#endif
