// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SEND_INFO_REQUEST_ACTION_H
#define SEND_INFO_REQUEST_ACTION_H

#include "common/action.h"
#include "tracker/validationManager.h"
#include "common/requestHandler.h"
#include "configureNodeActionHadler.h"

namespace client
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
		, MinPrice
		, MaxPrice
	};
};

extern std::vector< TrackerInfo::Enum > const TrackerDescription;

class CSendInfoRequestAction : public common::CAction< NodeResponses >
{
public:
	CSendInfoRequestAction( NetworkInfo::Enum const _networkInfo );

	virtual void accept( common::CSetResponseVisitor< NodeResponses > & _visitor );

	common::CRequest< NodeResponses >* execute();

private:
	common::CRequest< NodeResponses >* m_request;

	common::ActionStatus::Enum m_status;
};

struct CTrackersInfoRequest : public common::CRequest< NodeResponses >
{
public:
	CTrackersInfoRequest( std::vector< TrackerInfo::Enum > const & _reqInfo = std::vector< TrackerInfo::Enum >(), int _mediumKind = -1 );
	~CTrackersInfoRequest(){};
	void accept( common::CMedium< NodeResponses > * _medium ) const;
	common::CMediumFilter< TrackerResponses > * getMediumFilter() const;

	std::vector< TrackerInfo::Enum >const  m_reqInfo;
	int m_mediumKind;
};

struct CMonitorInfoRequest : public common::CRequest< NodeResponses >
{
public:
	CMonitorInfoRequest();
	void serialize( CBufferAsStream & _bufferStream ) const;
	int getMediumFilter() const;
};

struct CInfoRequestContinue : public common::CRequest< NodeResponses >
{
public:
	CInfoRequestContinue( uint256 const & _token, common::RequestKind::Enum const _requestKind );
	void accept( common::CMedium< NodeResponses > * _medium ) const;
	int getMediumFilter() const;

	uint256 const m_token;
	common::RequestKind::Enum const m_requestKind;
};

}

#endif // SEND_INFO_REQUEST_ACTION_H
