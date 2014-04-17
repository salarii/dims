// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ACTION_HANDLER_H
#define ACTION_HANDLER_H

#include <boost/thread.hpp>

#include <map>

#include "connectionProvider.h"

#include "action.h"

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

class CActionHandler
{
public:
	typedef std::map<RequestKind::Enum, CRequestHandler * > AvailableHandlers;
	typedef std::map< CRequest*, CAction* > RequestToAction;
public:
	void loop();
	void shutDown();
	~CActionHandler();
	static CActionHandler* getInstance( );

	void executeAction( CAction* _action );

	void addConnectionProvider( CConnectionProvider* _connectionProvider );
private:
	CActionHandler();

	CRequestHandler * provideHandler( RequestKind::Enum const _request );

    void findAction( CAction* _action ) const;
private:
	static CActionHandler * ms_instance;

	mutable boost::mutex m_mutex;

	std::list< CAction* > m_actions;

	RequestToAction m_reqToAction;

	std::list<CConnectionProvider*> m_connectionProviders;

	//this  will be  multimap one  day, this  should  be  periodically  cleanuped ,  don't  know  how  yet
	AvailableHandlers m_requestHandlers;
	
	static unsigned int const m_sleepTime;
};

}


#endif
