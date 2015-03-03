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
#include "configureClientActionHadler.h"

#include <exception>

#include "controlRequests.h"

class CBufferAsStream;

namespace client
{

class CNetworkClient : public common::CMedium< ClientResponses >, public QThread
{
public:
	CNetworkClient( QString const & _ipAddr,ushort const _port );

	~CNetworkClient();
	virtual void startThread();

	bool serviced() const throw(common::CMediumException);

	void add( common::CRequest< ClientResponses > const * _request );

	void add( CBalanceRequest const * _request );

	void add( CTransactionSendRequest const * _request );

	void add( CTransactionStatusRequest const * _request );

	void add( CInfoRequestContinue const * _request );

	void add( CRecognizeNetworkRequest const * _request );

	void add( CTrackersInfoRequest const * _request );

	void add( CMonitorInfoRequest const * _request );

	bool flush();

	virtual bool getResponseAndClear( std::multimap< common::CRequest< ClientResponses >const*, ClientResponses > & _requestResponse );
private:
	void clearResponses();

	void run();

	unsigned int read();

	int waitForInput();

	void write();

	common::CRequest< ClientResponses >* takeMatching( uint256 const & _token );

	bool processSomething() const;
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

	std::map< uint256, common::CRequest< ClientResponses >* > m_matching;

	std::list< common::CRequest< ClientResponses >* > m_workingRequest;

	int64_t const m_sleepTime;
};


}

#endif
