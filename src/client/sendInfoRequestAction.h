// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SEND_INFO_REQUEST_ACTION_H
#define SEND_INFO_REQUEST_ACTION_H

#include "common/action.h"
#include "tracker/validationManager.h"
#include "common/requestHandler.h"
#include "configureClientActionHadler.h"

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

class CSendInfoRequestAction : public common::CAction< ClientResponses >
{
public:
	CSendInfoRequestAction( NetworkInfo::Enum const _networkInfo );

	virtual void accept( common::CSetResponseVisitor< ClientResponses > & _visitor );
};

struct CTrackersInfoRequest : public common::CRequest< ClientResponses >
{
public:
	CTrackersInfoRequest( common::CMediumFilter< ClientResponses > * _mediumFilter );

	~CTrackersInfoRequest(){};

	void accept( common::CMedium< ClientResponses > * _medium ) const;

	common::CMediumFilter< ClientResponses > * getMediumFilter() const;

	int m_mediumKind;
};

struct CMonitorInfoRequest : public common::CRequest< ClientResponses >
{
public:
	CMonitorInfoRequest( common::CMediumFilter< ClientResponses > * _mediumFilter );

	void serialize( CBufferAsStream & _bufferStream ) const;

	void accept( common::CMedium< ClientResponses > * _medium ) const;
};

}

#endif // SEND_INFO_REQUEST_ACTION_H
