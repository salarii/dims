// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef NETWORK_CLIENT_H
#define NETWORK_CLIENT_H

#include "QMutex"
#include "QThread"
#include "QTcpSocket"
#include "common/communicationBuffer.h"
#include "common/medium.h"

#include <exception>

#include "controlRequests.h"

class CBufferAsStream;

namespace client
{

class CNetworkClient : public common::CMedium, public QThread
{
public:
	CNetworkClient( QString const & _ipAddr,ushort const _port );

	~CNetworkClient();

	bool serviced() const throw(common::CMediumException);

	void prepareMedium();

	void add( common::CRequest const * _request );

	void add( common::CSendClientMessageRequest const * _request );

	void add( client::CCreateTransactionRequest const * _request );

	bool flush();

	virtual bool getResponseAndClear( std::multimap< common::CRequest const*, common::DimsResponse > & _requestResponse );
private:
	void clearResponses();

	void run();

	unsigned int read();

	unsigned int waitForInput();

	void write();

	common::CRequest* takeMatching( uint256 const & _token );

	bool dropConnection() const;
private:
	static unsigned const m_timeout;

	mutable QMutex m_mutex;
	const QString m_ip;
	const ushort m_port;

	CBufferAsStream * m_pushStream;
// in prototype i split  those two buffer but most probably they could be merged to one
	common::CCommunicationBuffer m_pushBuffer;
	common::CCommunicationBuffer m_pullBuffer;

	QTcpSocket * m_socket;

	std::map< uint256, common::CRequest* > m_matching;

	std::list< common::CRequest* > m_workingRequest;

	int64_t const m_sleepTime;
};


}

#endif
