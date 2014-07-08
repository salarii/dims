// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef INTERNAL_MEDIUM_PROVIDER_H
#define INTERNAL_MEDIUM_PROVIDER_H

#include "common/connectionProvider.h"
#include "common/medium.h"
namespace tracker
{

class CBitcoinNodeMedium;

class CInternalMediumProvider : public  common::CConnectionProvider< TrackerResponses >
{
public:
	virtual std::list< common::CMedium< TrackerResponses > *> provideConnection( int const _actionKind, unsigned _requestedConnectionNumber = -1 );

	// set response, merkle ?? transaction ??
	void setResponse( CTransaction const & _response, CNode * _node );

	void setResponse( CMerkleBlock const & _merkle, CNode * _node );

	static CInternalMediumProvider* getInstance( );
private:
	CInternalMediumProvider();
private:
	mutable boost::mutex m_mutex;

	static CInternalMediumProvider * ms_instance;

	std::list< common::CMedium< TrackerResponses > *> m_mediums;
	// this is simplified approach
	std::map< CNode *, CBitcoinNodeMedium * > m_nodeToMedium;
};

}

#endif // INTERNAL_MEDIUM_PROVIDER_H
