// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ACTION_HANDLER_H
#define ACTION_HANDLER_H

#include <QThread>

#include <QMutex>

#include <map>


namespace node
{

class CSetResponseVisitor;
class CRequestHandler;
struct CRequest;

class CAction
{
public:
	virtual void accept( CSetResponseVisitor & _visitor );

	virtual CRequest* execute() = 0;
};

class CActionHandler : public QThread
{
public:
	void run();
	void shutDown();
private:
	QMutex m_mutex;
	std::list< CAction* > m_actions;

	std::map< CRequest*, CAction* > m_reqToAction;

	CRequestHandler * m_requestHandler;

	static unsigned int const m_sleepTime;
};

}

#endif