#ifndef ACTION_HANDLER_H
#define ACTION_HANDLER_H

namespace node
{

typedef boost::variant< CTransactionStatus, CAccountBalance > RequestRespond;

class CAction
{
public:
	virtual void accept( CSetResponseVisitor & _visitor );

	virtual CRequest* execute() = 0;
};

template < class T >
accept( T _visitor )

class CActionHandle : public QThread
{
public:

	void run();

private:
	QMutex m_mutex;
	std::list< CAction* > m_actions;

	std::map< CRequest*, CAction* > m_reqToAction;

	CRequestHandler * m_requestHandler;

	static unsigned int const m_sleepTime;
};

}

#endif