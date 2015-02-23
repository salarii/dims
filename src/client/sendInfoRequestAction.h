// Copyright (c) 2014-2015 Dims dev-team
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

	common::CRequest< NodeResponses >* getRequest() const;

private:
	common::CRequest< NodeResponses >* m_request;
};

struct CTrackersInfoRequest : public common::CRequest< NodeResponses >
{
public:
	CTrackersInfoRequest( common::CMediumFilter< NodeResponses > * _mediumFilter );
	~CTrackersInfoRequest(){};
	void accept( common::CMedium< NodeResponses > * _medium ) const;
	common::CMediumFilter< NodeResponses > * getMediumFilter() const;

	int m_mediumKind;
};

struct CMonitorInfoRequest : public common::CRequest< NodeResponses >
{
public:
	CMonitorInfoRequest( common::CMediumFilter< NodeResponses > * _mediumFilter );
	void serialize( CBufferAsStream & _bufferStream ) const;
	void accept( common::CMedium< NodeResponses > * _medium ) const;
};

struct CInfoRequestContinueComplex : public common::CRequest< NodeResponses >
{
public:
	CInfoRequestContinueComplex( std::map< uintptr_t, uint256 > const & _nodeToToken, common::CMediumFilter< NodeResponses > * _mediumFilter );
	void accept( common::CMedium< NodeResponses > * _medium ) const;
	common::CMediumFilter< NodeResponses > * getMediumFilter() const;

	std::map< uintptr_t, uint256 > const & m_nodeToToken;
};

}

#endif // SEND_INFO_REQUEST_ACTION_H
