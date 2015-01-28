// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef NETWORK_CLIENT_H
#define NETWORK_CLIENT_H

#include "QMutex"
#include "QThread"
#include "QTcpSocket"
#include "common/communicationBuffer.h"
#include "common/medium.h"
#include "configureNodeActionHadler.h"

#include <exception>

#include "controlRequests.h"

class CBufferAsStream;

namespace client
{

class CNetworkClient : public common::CMedium< NodeResponses >, public QThread
{
public:
    enum ConnectionInfo
    {
          ServiceDenial
        , NoResponse
        , NoActivity
        , Processing
        , Processed
    };

public:
	CNetworkClient( QString const & _ipAddr,ushort const _port );

    ~CNetworkClient();
	virtual void startThread();

	bool serviced() const throw(common::CMediumException);

	void add( common::CRequest< NodeResponses > const * _request );

	void add( CBalanceRequest const * _request );

	void add( CTransactionSendRequest const * _request );

	void add( CTransactionStatusRequest const * _request );

	void add( CInfoRequestContinueComplex const * _request );

	void add( CInfoRequestContinue const * _request );

	void add( CRecognizeNetworkRequest const * _request );

	void add( CTrackersInfoRequest const * _request );

	void add( CMonitorInfoRequest const * _request );

	bool flush();

	void clearResponses();

	virtual bool getResponse( std::vector< NodeResponses > & _requestResponse ) const;
private:
	void run();
	unsigned int read();
	int waitForInput();
	void write();
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

    ConnectionInfo m_connectionInfo;
};


}

#endif
