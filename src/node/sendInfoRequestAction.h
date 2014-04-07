// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SEND_INFO_REQUEST_ACTION_H
#define SEND_INFO_REQUEST_ACTION_H

#include "actionHandler.h"
#include "tracker/validationManager.h"
#include "requestHandler.h"

namespace node
{

struct TrackerInfo
{
	enum Enum
	{
		  Ip
		, Price
		, Rating
	};
};

class CSetResponseVisitor;

class CSendInfoRequestAction : public CAction
{
public:
	CSendInfoRequestAction();

	void accept( CSetResponseVisitor & _visitor );
private:
};

struct CTrackersInfoRequest : public CRequest
{
public:
	CTrackersInfoRequest( std::vector< TrackerInfo::Enum >& const _reqInfo = std::vector< TrackerInfo::Enum >() );
	void serialize( CBufferAsStream & _bufferStream ) const;

	std::vector< TrackerInfo::Enum > m_reqInfo;
};

struct CMonitorInfoRequest : public CRequest
{
public:
	CMonitorInfoRequest();
	void serialize( CBufferAsStream & _bufferStream ) const;
};


}

#endif // SEND_INFO_REQUEST_ACTION_H