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
	CTrackersInfoRequest( std::vector< TrackerInfo::Enum > const & _reqInfo, common::CMediumFilter< NodeResponses > * _mediumFilter );
	~CTrackersInfoRequest(){};
	void accept( common::CMedium< NodeResponses > * _medium ) const;
	common::CMediumFilter< NodeResponses > * getMediumFilter() const;

	std::vector< TrackerInfo::Enum >const  m_reqInfo;
	int m_mediumKind;
};

struct CMonitorInfoRequest : public common::CRequest< NodeResponses >
{
public:
	CMonitorInfoRequest();
	void serialize( CBufferAsStream & _bufferStream ) const;
	common::CMediumFilter< NodeResponses > * getMediumFilter() const;
};

struct CInfoRequestContinueComplex : public common::CRequest< NodeResponses >
{
public:
	CInfoRequestContinueComplex( std::map< uintptr_t, uint256 > const & _nodeToToken, common::CMediumFilter< NodeResponses > * _mediumFilter );
	void accept( common::CMedium< NodeResponses > * _medium ) const;
	common::CMediumFilter< NodeResponses > * getMediumFilter() const;

	std::map< uintptr_t, uint256 > const & m_nodeToToken;
};

struct CInfoRequestContinue : public common::CRequest< NodeResponses >
{
public:
	CInfoRequestContinue( uint256 const & _token, common::CMediumFilter< NodeResponses > * _mediumFilter );
	void accept( common::CMedium< NodeResponses > * _medium ) const;
	common::CMediumFilter< NodeResponses > * getMediumFilter() const;

	uint256 const m_token;
};

}

#endif // SEND_INFO_REQUEST_ACTION_H
