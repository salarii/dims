// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


#ifndef SEED_NODES_MANAGER_H
#define SEED_NODES_MANAGER_H

#include "common/nodesManager.h"
#include "common/communicationProtocol.h"
#include "common/connectionProvider.h"
#include "configureSeedActionHandler.h"

namespace seed
{

class CSeedNodeMedium;

class CSeedNodesManager : public common::CNodesManager< SeedResponses >
{
public:
	static CSeedNodesManager * getInstance();

	CSeedNodeMedium* getMediumForNode( common::CSelfNode * _node ) const;

	std::list< common::CMedium< SeedResponses > *> provideConnection( common::CMediumFilter< SeedResponses > const & _mediumFilter );

	void setPublicKey( CAddress const & _address, CPubKey const & _pubKey );

	bool getPublicKey( CAddress const & _address, CPubKey & _pubKey ) const;

	std::list< common::CMedium< SeedResponses > *> getInternalMedium();
private:
	CSeedNodesManager();
private:
	std::map< CAddress, CPubKey > m_keyStore;
};





}

#endif // SEED_NODES_MANAGER_H
