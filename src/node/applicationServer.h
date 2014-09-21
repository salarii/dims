// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef APPLICATION_SERVER_H
#define APPLICATION_SERVER_H

#include <QLocalServer>
#include <QLocalSocket>

#include "common/medium.h"
#include "configureNodeActionHadler.h"

namespace client
{

class CLocalSocket : public common::CMedium< NodeResponses >
{
public:
	CLocalSocket( QLocalSocket * _localSocket );

	~CLocalSocket();

	bool serviced() const throw(common::CMediumException);

	bool flush();

	void handleInput();

	void clearResponses();

	bool getResponse( std::vector< NodeResponses > & _requestResponse ) const;

	QLocalSocket * getSocket() const;
protected:
	QLocalSocket * m_localSocket;
};


class CLocalServer :  public QObject
{
	Q_OBJECT
public:
	~CLocalServer();

	static CLocalServer* getInstance();
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
