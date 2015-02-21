// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef APPLICATION_SERVER_H
#define APPLICATION_SERVER_H

#include <QLocalServer>
#include <QLocalSocket>

#include "common/medium.h"
#include "common/connectionProvider.h"

#include "configureNodeActionHadler.h"

namespace client
{

class CLocalSocket : public common::CMedium< NodeResponses >
{
public:
	CLocalSocket( QLocalSocket * _localSocket );

	~CLocalSocket();

	bool serviced() const throw(common::CMediumException);

	void add( CErrorForAppPaymentProcessing const * _request );

	void add( CProofTransactionAndStatusRequest const * _request );

	bool flush();

	void handleInput();

	bool getResponseAndClear( std::vector< NodeResponses > & _requestResponse );

	QLocalSocket * getSocket() const;
protected:
	// add locking
	void clearResponses();

	QLocalSocket * m_localSocket;

	std::vector< NodeResponses > m_nodeResponses;
};


class CLocalServer :  public QObject, public common::CConnectionProvider< NodeResponses >
{
	Q_OBJECT
public:
	~CLocalServer();

	std::list< common::CMedium< NodeResponses > *> provideConnection( common::CMediumFilter< NodeResponses > const & _mediumFilter );

	static CLocalServer* getInstance();

	bool getSocked( uintptr_t _ptr, CLocalSocket *& _localSocket ) const;
private slots:

	void newConnection();

	void readSocket();

	void discardSocket();
private:
	CLocalServer();
private:
	static CLocalServer * ms_instance;

	QLocalServer m_server;

	std::map< uintptr_t, CLocalSocket* > m_usedSockts;

	void addToList( CLocalSocket* _socket );
};

}

#endif // APPLICATION_SERVER_H
