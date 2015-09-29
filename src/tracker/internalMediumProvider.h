// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef INTERNAL_MEDIUM_PROVIDER_H
#define INTERNAL_MEDIUM_PROVIDER_H

#include "common/connectionProvider.h"
#include "common/medium.h"
#include "common/mediumKinds.h"

namespace common
{

class CBitcoinNodeMedium;

}

namespace tracker
{

class CInternalMediumProvider : public  common::CConnectionProvider
{
public:
	virtual std::list< common::CMedium *> provideConnection( common::CMediumFilter const & _mediumFilter );

	// set response, merkle ?? transaction ??
	void setTransaction( CTransaction const & _response, CNode * _node );

	void setMerkleBlock( CMerkleBlock const & _merkle, CNode * _node );

	std::list< common::CMedium *> getMediumByClass( common::CMediumKinds::Enum _mediumKind, unsigned int _mediumNumber );

	static CInternalMediumProvider* getInstance( );

	void registerRemoveCallback( CNodeSignals& nodeSignals );

	void stopCommunicationWithNode( uintptr_t _nodePtr );
private:
	void removeNodeCallback( CNode * node );

	CInternalMediumProvider();
private:
	mutable boost::mutex m_mutex;

	static CInternalMediumProvider * ms_instance;
	// this is simplified approach
	std::map< CNode *, common::CBitcoinNodeMedium * > m_nodeToMedium;
};

}

#endif // INTERNAL_MEDIUM_PROVIDER_H
