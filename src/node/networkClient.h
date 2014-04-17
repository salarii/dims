// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef NETWORK_CLIENT_H
#define NETWORK_CLIENT_H

#include "QMutex"
#include "QThread"
#include "QTcpSocket"
#include "medium.h"
#include "common/communicationBuffer.h"

class CBufferAsStream;

namespace node
{
// in out 

class CNetworkClient : public QThread, public CMedium
{
public:
	CNetworkClient( QString const & _ipAddr,ushort const _port );

    ~CNetworkClient();
    virtual void startThread();
	virtual void stopThread();

	bool serviced() const;
	void add( CRequest const * _request );
	bool flush();
	bool getResponse( common::CCommunicationBuffer & _outBuffor ) const;
private:
	void setRunThread( bool newVal );
	bool getRunThread();
	void run();
	unsigned int read();
	int waitForInput();
	void write();
private:
	bool mRunThread;
	static unsigned const m_timeout;

    QMutex m_mutex;
    QMutex m_writeMutex;
	const QString m_ip;
	const ushort m_port;

    bool m_flushBuffor;
	CBufferAsStream * m_pushStream;
// in prototype i split  those two buffer but most probably they could be merged to one
	common::CCommunicationBuffer m_pushBuffer;
	common::CCommunicationBuffer m_pullBuffer;

	QTcpSocket * m_socket;
};


}

#endif
