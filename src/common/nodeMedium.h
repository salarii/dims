// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef NODE_MEDIUM_H
#define NODE_MEDIUM_H

#include <boost/variant.hpp>

#include "common/medium.h"
#include "common/selfNode.h"
#include "common/requests.h"

namespace common
{
typedef boost::variant< common::CIdentifyMessage > ProtocolMessage;

class CSegmentHeader;
struct CDiskBlock;

class CNodeMedium : public CMedium
{
public:
	CNodeMedium( common::CSelfNode * _selfNode ):m_usedNode( _selfNode ){};

	bool serviced() const;

	bool flush();

	bool getResponseAndClear( std::multimap< CRequest const*, DimsResponse > & _requestResponse );

	void add( common::CRequest const * _request );

	void add( CSendIdentifyDataRequest const * _request );

	void add( CAckRequest const * _request );

	void add( CInfoAskRequest const * _request );

	void add( CSendMessageRequest const * _request );

	void setResponse( uint256 const & _id, DimsResponse const & _responses );

	void deleteRequest( CRequest const* _request );

	common::CSelfNode * getNode() const;

	bool getDirectActionResponseAndClear( CAction const * _action, std::list< DimsResponse > & _responses );

	void addActionResponse( uint256 const & _actionId, DimsResponse const & _response );
protected:
	void clearResponses();

	void setLastRequest( uint256 const & _id, common::CRequest const* _request );
protected:
	common::CSelfNode * m_usedNode;

	mutable boost::mutex m_mutex;
	std::multimap< uint256, DimsResponse > m_responses;

	static uint256 m_counter;

	std::vector< common::CMessage > m_messages;

	std::set< uint256 > m_indexes;

	std::list< uint256 > m_synchronizeQueue;

	std::map< uint256, common::CRequest const* > m_idToRequest;

	std::multimap< uint256, DimsResponse > m_actionToResponse;
};


}

#endif // NODE_MEDIUM_H
