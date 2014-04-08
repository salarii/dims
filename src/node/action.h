// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ACTION_H
#define ACTION_H

namespace node
{

class CSetResponseVisitor;
struct CRequest;

struct ActionStatus
{
	enum Enum
	{
		Unprepared
		,InProgress
	};
};

class CAction
{
public:
	virtual void accept( CSetResponseVisitor & _visitor );

	virtual CRequest* execute() = 0;
};

}

#endif