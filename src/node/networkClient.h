// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef NETWORK_CLIENT_H
#define NETWORK_CLIENT_H

#define MaxBufferSize 1000

#include "QMutex"
#include "QThread"
#include "QTcpSocket"

namespace node
{
// in out 
struct CCommunicationBuffer
{
	char m_buffer[ MaxBufferSize ];
	unsigned int m_usedSize;
};

class CNetworkClient : public QThread
{
public:
	CNetworkClient( QString const & _ipAddr,ushort const _port );
	virtual void startThread();
	virtual void stopThread();

	void send( CCommunicationBuffer const & _inBuffor );
	bool serviced() const;
	bool getResponse( CCommunicationBuffer & _outBuffor ) const;
private:
	QMutex m_mutex;
	const QString m_ip;
	const ushort m_port;

	void setRunThread( bool newVal );
	bool getRunThread();
	void run();
	unsigned int read(QTcpSocket *socket );
	int waitForInput( QTcpSocket *socket );
	void write( QTcpSocket *client );
private:
	bool mRunThread;
	static unsigned const m_timeout;

	CCommunicationBuffer m_inBuffor;
	CCommunicationBuffer m_outBuffor;
};


}

#endif