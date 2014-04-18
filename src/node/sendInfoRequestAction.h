// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SEND_INFO_REQUEST_ACTION_H
#define SEND_INFO_REQUEST_ACTION_H

#include "action.h"
#include "tracker/validationManager.h"
#include "requestHandler.h"


namespace node
{


struct NetworkInfo
{
	enum Enum
	{
		  Monitor
		, Tracker
	};
};

struct TrackerInfo
{
	enum Enum
	{
		  Ip
		, Price
		, Rating
        , PublicKey
        , Port
	};
};

extern std::vector< TrackerInfo::Enum > const TrackerDescription;

class CSetResponseVisitor;

class CSendInfoRequestAction : public CAction
{
public:
	CSendInfoRequestAction( NetworkInfo::Enum const _networkInfo );

	void accept( CSetResponseVisitor & _visitor );

	CRequest* execute();

private:
	CRequest* m_request;

	ActionStatus::Enum m_status;
};

struct CTrackersInfoRequest : public CRequest
{
public:
	CTrackersInfoRequest( std::vector< TrackerInfo::Enum > const & _reqInfo = std::vector< TrackerInfo::Enum >() );
	~CTrackersInfoRequest(){};
	void serialize( CBufferAsStream & _bufferStream ) const;
	RequestKind::Enum getKind() const;

	std::vector< TrackerInfo::Enum >const  m_reqInfo;
};

struct CMonitorInfoRequest : public CRequest
{
public:
	CMonitorInfoRequest();
	void serialize( CBufferAsStream & _bufferStream ) const;
	RequestKind::Enum getKind() const;
};

struct CInfoRequestContinue : public CRequest
{
public:
	CInfoRequestContinue( uint256 const & _token, RequestKind::Enum const _requestKind );
	void serialize( CBufferAsStream & _bufferStream ) const;
	RequestKind::Enum getKind() const;

	uint256 const m_token;
	RequestKind::Enum const m_requestKind;
};

}

#endif // SEND_INFO_REQUEST_ACTION_H
