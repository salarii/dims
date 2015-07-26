// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef INTERNAL_MEDIUM_PROVIDER_H
#define INTERNAL_MEDIUM_PROVIDER_H

#include "common/connectionProvider.h"
#include "common/medium.h"
#include "common/mediumKinds.h"

namespace common
{

template < class _Type >
class CBitcoinNodeMedium;

}

namespace tracker
{

class CInternalMediumProvider : public  common::CConnectionProvider< common::CTrackerTypes >
{
public:
	virtual std::list< common::CTrackerBaseMedium *> provideConnection( common::CTrackerMediumFilter const & _mediumFilter );

	// set response, merkle ?? transaction ??
	void setTransaction( CTransaction const & _response, CNode * _node );

	void setMerkleBlock( CMerkleBlock const & _merkle, CNode * _node );

	std::list< common::CTrackerBaseMedium *> getMediumByClass( common::CMediumKinds::Enum _mediumKind, unsigned int _mediumNumber );

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
	std::map< CNode *, common::CBitcoinNodeMedium< common::CTrackerTypes > * > m_nodeToMedium;
};

}

#endif // INTERNAL_MEDIUM_PROVIDER_H
