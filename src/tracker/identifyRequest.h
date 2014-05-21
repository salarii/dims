// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef IDENTIFY_REQUEST_H
#define IDENTIFY_REQUEST_H

#include "common/request.h"
#include "net.h"
#include "configureTrackerActionHandler.h"
#include "common/medium.h"

#include "trackerMediumsKinds.h"
#include "util.h"

namespace tracker
{

class CIdentifyRequest : public common::CRequest< TrackerResponses >
{
public:
	CIdentifyRequest( CSelfNode * _node, std::vector< unsigned char > const & _payload );

	void accept( common::CMedium< TrackerResponses > * _medium ) const;

	int getKind() const;

	std::vector< unsigned char > getPayload() const;

	CSelfNode * getNode() const;
private:

	std::vector< unsigned char > m_payload;

	CSelfNode * m_node;
};

class CIdentifyResponse : public common::CRequest< TrackerResponses >
{
public:
	CIdentifyResponse( CSelfNode * _node, std::vector< unsigned char > const & _signed, uint160 _keyId );

	void accept( common::CMedium< TrackerResponses > * _medium ) const;

	int getKind() const;

	CSelfNode * getNode() const;

	std::vector< unsigned char > getSigned() const;

	uint160 getKeyID() const;
private:
	CSelfNode * m_node;
	std::vector< unsigned char > m_signed;
	uint160 m_keyId;
};

}

#endif // IDENTIFY_REQUEST_H
