// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ACTION_HANDLER_H
#define ACTION_HANDLER_H

#include <boost/thread.hpp>

namespace common
{

class CSetResponseVisitor;
class CRequestHandler;
class CRequest;
class CConnectionProvider;
class CAction;
class CMedium;
class CMediumFilter;

struct LessHandlers : public std::binary_function< CRequestHandler * ,CRequestHandler * ,bool>
{
	bool operator() ( CRequestHandler * const & _handlerLhs, CRequestHandler * const & _handlerRhs) const;

	bool operator() ( CRequestHandler * const & _handlerLhs, CMedium* const & _medium) const;

	bool operator() ( CMedium* const & _medium, CRequestHandler * const & _handlerLhs ) const;
};

class CActionHandler
{
public:
	typedef std::set< CRequestHandler *, LessHandlers > AvailableHandlers;

	typedef std::map< CRequest*, CAction * > RequestToAction;

	typedef std::multimap< CRequest*, CRequestHandler * > RequestToHandlers;
public:
	void loop();

	void shutDown();

	~CActionHandler();

	static CActionHandler* getInstance( );

	void executeAction( CAction * _action );

	void addConnectionProvider( CConnectionProvider * _connectionProvider );
private:
	CActionHandler();

	std::list< CRequestHandler * > provideHandler( CMediumFilter const & _filter );

	void findAction( CAction * _action ) const;

private:
	static CActionHandler * ms_instance;

	mutable boost::mutex m_mutex;

	std::set< CAction * > m_actions;

	RequestToAction m_reqToAction;

	std::map< CAction *, std::set< CRequest* > > m_actionToExecutedRequests;

	std::list<CConnectionProvider*> m_connectionProviders;

	AvailableHandlers m_requestHandlers;

	static unsigned int const m_sleepTime;

	RequestToHandlers m_currentlyUsedHandlers;// clean this  up when an action dies
};


}


#endif
