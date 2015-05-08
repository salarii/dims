// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef INTERNAL_MEDIUM_PROVIDER_H
#define INTERNAL_MEDIUM_PROVIDER_H

#include "common/connectionProvider.h"
#include "common/medium.h"
#include "common/mediumKinds.h"

namespace monitor
{

class CBitcoinNodeMedium;

class CInternalMediumProvider : public  common::CConnectionProvider< common::CMonitorTypes >
{
public:
	virtual std::list< common::CMonitorBaseMedium *> provideConnection( common::CMonitorMediumFilter const & _mediumFilter );

	std::list< common::CMonitorBaseMedium *> getMediumByClass( common::CMediumKinds::Enum _mediumKind, unsigned int _mediumNumber );

	void setResponse( CTransaction const & _response, CNode * _node );

	void setResponse( CMerkleBlock const & _merkle, CNode * _node );

	static CInternalMediumProvider* getInstance( );

	void registerRemoveCallback( CNodeSignals& nodeSignals );
private:
	void removeNodeCallback( CNode * node );

	CInternalMediumProvider();
private:
	mutable boost::mutex m_mutex;

	static CInternalMediumProvider * ms_instance;

	// this is simplified approach
	std::map< CNode *, CBitcoinNodeMedium * > m_nodeToMedium;
};

}

#endif // INTERNAL_MEDIUM_PROVIDER_H
