// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef NODE_MEDIUM_H
#define NODE_MEDIUM_H

#include "common/medium.h"
#include "communicationProtocol.h"
#include <boost/variant.hpp>
namespace tracker
{

typedef boost::variant< CIdentifyMessage > ProtocolMessage;

class CIdentifyRequest;
//I don't know  how to identify which what response is to what  request  yet
class CNodeMedium : public common::CMedium< TrackerResponses >
{
public:
	CNodeMedium( CSelfNode * _selfNode ):m_usedNode( _selfNode ){};

	bool serviced() const;

	bool flush();

	bool getResponse( std::vector< TrackerResponses > & _requestResponse ) const;

	void clearResponses();

	void add( common::CRequest< TrackerResponses > const * _request );

	void add( CIdentifyRequest const * _request );

	void add( CIdentifyResponse const * _request );

	void add( CContinueReqest const * _request );

	void setResponse( uint256 const & _id, TrackerResponses const & _responses );

	bool isIdentifyMessageKnown( uint256 const & _payloadHash ) const;

	CSelfNode * getNode() const;
private:
	CSelfNode * m_usedNode;

	mutable boost::mutex m_mutex;
	std::map< uint256, TrackerResponses > m_responses;

	std::set< uint256 > m_findIdentifyMessage;

	static uint256 m_counter;

	std::vector< CMessage > m_messages;

	std::vector< uint256 > m_indexes;

};

}

#endif // NODE_MEDIUM_H
