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
	CIdentifyRequest( unsigned int _kind, std::vector< unsigned char > const & _payload );

	void accept( common::CMedium< TrackerResponses > * _medium ) const;

	int getKind() const;

	std::vector< unsigned char > getPayload() const;
private:

	std::vector< unsigned char > m_payload;

	unsigned int m_kind;
};

class CIdentifyResponse : public common::CRequest< TrackerResponses >
{
public:
	CIdentifyResponse( unsigned int _kind, std::vector< unsigned char > const & _signed, uint160 _keyId, std::vector< unsigned char > const & _payload );

	void accept( common::CMedium< TrackerResponses > * _medium ) const;

	int getKind() const;


	std::vector< unsigned char > getSigned() const;

	uint160 getKeyID() const;

	std::vector< unsigned char > getPayload()const;
private:
	unsigned int m_kind;
	std::vector< unsigned char > m_signed;
	uint160 m_keyId;
	std::vector< unsigned char > m_payload;
};

}

#endif // IDENTIFY_REQUEST_H