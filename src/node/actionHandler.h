// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ACTION_HANDLER_H
#define ACTION_HANDLER_H

#include <boost/thread.hpp>

#include <map>

#include "connectionProvider.h"

namespace node
{
/*
connect  with  seed
connect  with  monitor or  tracker
rebuild reputation  tables
handle  requests
rebuild  reputation  table  periodically
?? put  this as permenent  action  with  counter ?? action 
*/
class CSetResponseVisitor;
class CRequestHandler;
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

class CActionHandler
{
public:
	void loop();
	void shutDown();
	~CActionHandler();
	static CActionHandler* getInstance( );

	void executeAction( CAction* _action );
private:
	CActionHandler();

	CRequestHandler * provideHandler( RequestKind::Enum const _request );
private:
	static CActionHandler * ms_instance;

	mutable boost::mutex m_mutex;

	std::list< CAction* > m_actions;

	std::map< CRequest*, CAction* > m_reqToAction;

	CRequestHandler * m_requestHandler;

	std::list<CConnectionProvider*> m_connectionProviders;

	//this  will be  multimap one  day, this  should  be  periodically  cleanuped ,  don't  know  how  yet
	std::map<RequestKind::Enum, CRequestHandler * > m_requestHandlers;
	
	static unsigned int const m_sleepTime;
};

}


#endif