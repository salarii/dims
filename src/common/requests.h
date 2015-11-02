// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef REQUESTS_H
#define REQUESTS_H

#include "uint256.h"
#include "key.h"
#include "protocol.h"

#include "common/request.h"
#include "common/communicationProtocol.h"
#include "common/nodeMessages.h"
#include "common/support.h"

// rename to  common requests??
namespace common
{

struct CBlockKind
{
	enum Enum
	{
		Segment
		, Header
	};
};

class CSendIdentifyDataRequest : public common::CRequest
{
public:
	CSendIdentifyDataRequest( std::vector< unsigned char > const & _signed, CPubKey const & _key, std::vector< unsigned char > const & _payload, uint256 const & _actionKey, CMediumFilter * _mediumFilter );

	void accept( CMedium * _medium ) const;

	std::vector< unsigned char > getSigned() const;

	CPubKey getKey() const;

	std::vector< unsigned char > getPayload()const;

	uint256 getActionKey() const;
private:
	std::vector< unsigned char > m_signed;

	CPubKey m_key;

	std::vector< unsigned char > m_payload;

	uint256 const m_actionKey;
};

class CConnectToNodeRequest : public common::CRequest
{
public:
	CConnectToNodeRequest( std::string const & _trackerAddress, CAddress const & _serviceAddress, CMediumFilter * _mediumFilter );

	virtual void accept( CMedium * _medium ) const;

	std::string getAddress() const;

	CAddress getServiceAddress() const;
private:
	std::string const m_trackerAddress;

	CAddress const m_serviceAddress;
};
// turn  ack  into signed message???
class CAckRequest : public common::CRequest
{
public:
	CAckRequest( uint256 const & _actionKey, uint256 const & _id, CMediumFilter * _mediumFilter );

	virtual void accept( CMedium * _medium ) const;

	uint256 getActionKey() const;
private:
	uint256 const m_actionKey;
};

class CTimeEventRequest : public common::CRequest
{
public:
	CTimeEventRequest( int64_t _requestedDelay, CMediumFilter * _mediumFilter );

	virtual void accept( CMedium * _medium ) const;

	int64_t getEventTime() const;
private:
	unsigned int m_result;

	int64_t m_requestedDelay;
};

class CScheduleAbleAction;

class CScheduleActionRequest : public common::CRequest
{
public:
	CScheduleActionRequest( CScheduleAbleAction * _scheduleAbleAction, CMediumFilter * _mediumFilter );

	virtual void accept( CMedium * _medium ) const;

	CScheduleAbleAction * getAction() const;
private:
	CScheduleAbleAction * m_scheduleAbleAction;
};

class CAskForTransactionsRequest : public common::CRequest
{
public:
	CAskForTransactionsRequest( std::vector< uint256 > const & _blockHashes, CMediumFilter * _mediumFilter );

	virtual void accept( CMedium * m_mediumNumber ) const;

	std::vector< uint256 > const & getBlockHashes() const;
private:
	std::vector< uint256 > const m_blockHashes;

};

class CSetBloomFilterRequest : public common::CRequest
{
public:
	CSetBloomFilterRequest( CBloomFilter const & _bloomFilter, CMediumFilter * _CMediumFilter );

	virtual void accept( CMedium * _medium ) const;

	CBloomFilter const & getBloomFilter() const;
private:
	CBloomFilter const m_bloomFilter;
};

class CBalanceRequest : public common::CRequest
{
public:
	CBalanceRequest( CMediumFilter * _CMediumFilter );

	virtual void accept( CMedium * _medium ) const;

	uint160 getKey() const;
private:
	uint160 m_key;
};

class CSendClientMessageRequest: public common::CRequest
{
public:
	CSendClientMessageRequest(
			common::CMainRequestType::Enum _messageKind
			, CMediumFilter * _CMediumFilter );// bit  overuse of concept

	CSendClientMessageRequest(
			common::CMainRequestType::Enum _messageKind
			, uint256 const & _id
			, CMediumFilter * _CMediumFilter );// bit  overuse of concept

	void accept( CMedium * _medium ) const;

	int getMessageKind() const;

	std::vector< unsigned char > const & getPayLoad() const;

	//better  define it  here
	template < class T >
	void addPayload( T const & _t )
	{
		unsigned int initiaSize = m_payload.size();
		unsigned int size = ::GetSerializeSize( _t, SER_NETWORK, PROTOCOL_VERSION );
		m_payload.resize( size + initiaSize );
		CBufferAsStream stream( (char*)&m_payload[ initiaSize ], size, SER_NETWORK, PROTOCOL_VERSION );
		stream << _t;
	}
private:
	int m_messageKind;

	std::vector< unsigned char > m_payload;
};

class CSendMessageRequest: public common::CRequest
{
public:
	template < class Payload >
	CSendMessageRequest(
			CPayloadKind::Enum _messageKind
			, Payload const & _payload
			, uint256 const & _actionKey
			, uint256 const & _id
			, CMediumFilter * _CMediumFilter )
	: common::CRequest( _id, _CMediumFilter )
{
		common::CMessage message(
					_messageKind
					, createPayload( _payload )
					, _actionKey
					, _id );
}

	template < class Payload >
	CSendMessageRequest(
			CPayloadKind::Enum _messageKind
			, Payload const & _payload
			, uint256 const & _actionKey
			, CMediumFilter * _CMediumFilter )
	{
		common::CMessage message(
					_messageKind
					, createPayload( _payload )
					, _actionKey
					, getId() );
	}

	CSendMessageRequest(
			common::CMessage const & _message
			, CPubKey const & _prevKey
			, uint256 const & _actionKey
			, CMediumFilter * _CMediumFilter )
	{
		common::CMessage message(
					_message
					, _prevKey
					, _actionKey
					, getId() );
	}

	common::CMessage const & getMessage()const
	{
		return m_message;
	}

	void accept( CMedium * _medium ) const;
private:
	//better  define it  here
	template < class T >
	std::vector< unsigned char > createPayload( T const & _t )
	{
		std::vector< unsigned char > payload;
		unsigned int size = ::GetSerializeSize( _t, SER_NETWORK, PROTOCOL_VERSION );
		payload.resize( size );
		CBufferAsStream stream( (char*)&payload[ 0 ], size, SER_NETWORK, PROTOCOL_VERSION );
		stream << _t;
		return payload;
	}
private:
	common::CMessage m_message;
};

}

#endif // REQUESTS_H
